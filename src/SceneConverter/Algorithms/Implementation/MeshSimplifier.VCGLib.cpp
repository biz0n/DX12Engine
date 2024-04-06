#include <Algorithms/MeshSimplifier.h>

#include <vcg/simplex/face/topology.h>
#include <vcg/container/simple_temporary_data.h>
#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/clean.h>
#include <vcg/complex/algorithms/local_optimization.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric_tex.h>

class VCGVertex;
class VCGFace;
class VCGMesh;

class VCGUsedTypes : public vcg::UsedTypes<
	vcg::Use<VCGVertex>::AsVertexType,
	vcg::Use<VCGFace  >::AsFaceType
>
{
};

class VCGVertex : public vcg::Vertex<
	VCGUsedTypes,
	vcg::vertex::InfoOcf,
	vcg::vertex::Coord3f,
	vcg::vertex::BitFlags,
	vcg::vertex::Normal3f,
	vcg::vertex::VFAdjOcf,
	vcg::vertex::MarkOcf,
	vcg::vertex::TexCoordfOcf
>
{
};

class VCGFace : public vcg::Face<
	VCGUsedTypes,
	vcg::face::InfoOcf,
	vcg::face::VertexRef,
	vcg::face::BitFlags,
	vcg::face::Normal3f,
	vcg::face::MarkOcf,
	vcg::face::FFAdjOcf,
	vcg::face::VFAdjOcf,
	vcg::face::WedgeTexCoordfOcf
>
{
};

class VCGMesh : public vcg::tri::TriMesh<
	vcg::vertex::vector_ocf<VCGVertex>,
	vcg::face::vector_ocf<VCGFace>
>
{
};


void Simplify(VCGMesh& mesh, int targetFaceNum, double qualityThreshold, bool lockVertices, bool texSimplification, float& resultError);

namespace SceneConverter::Algorithms::MeshletSimplifier
{
    std::vector<uint32_t> VCGLibSimplify(
        const std::vector<uint32_t>& indices,
        const std::vector<Model::RawVertex>& vertices,
        const std::unordered_set<uint32_t>& boundaries,
		float threshold,
        float targetError,
        float& resultError)
    {
		constexpr bool lockVertices = false;
		constexpr bool texSimplification = false;

		VCGMesh mesh;
		mesh.vert.EnableTexCoord();
		mesh.vert.EnableVFAdjacency();
		mesh.vert.EnableMark();

		mesh.face.EnableWedgeTexCoord();
		mesh.face.EnableVFAdjacency();
		mesh.Clear();

		size_t nFaces = indices.size() / 3;
		vcg::tri::Allocator<VCGMesh>::AddVertices(mesh, vertices.size());
		vcg::tri::Allocator<VCGMesh>::AddFaces(mesh, nFaces);

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			const auto& vv = vertices[i];
			mesh.vert[i].P() = { vv.Position.x, vv.Position.y, vv.Position.z };
			mesh.vert[i].N() = { vv.Normal.x, vv.Normal.y, vv.Normal.z };
			mesh.vert[i].T() = { vv.TextureCoord.x, vv.TextureCoord.y };
		}

		for (size_t i = 0; i < nFaces; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				int32_t index = indices[i * 3 + j];
				mesh.face[i].V(j) = &mesh.vert[index];

				if (lockVertices)
				{
					if (boundaries.find(index) == boundaries.end())
					{
						mesh.vert[index].SetW();
					}
					else
					{
						mesh.vert[index].ClearW();
					}
				}
			}
		}

		

		Simplify(mesh, nFaces * threshold, targetError, lockVertices, texSimplification, resultError);

		std::vector<uint32_t> simplifiedIndices;
		for (auto fi = mesh.face.begin(); fi != mesh.face.end(); ++fi) 
		{
			if ((*fi).IsD())
			{
				continue;
			}

			for (int k = 0; k < 3; k++)
			{
				uint32_t index = vcg::tri::Index(mesh, (*fi).V(k));
				simplifiedIndices.push_back(index);
			}
		}

		return simplifiedIndices;
    }
}


typedef	vcg::SimpleTempData<VCGMesh::VertContainer, vcg::math::Quadric<double>> QuadricTemp;

class QuadricHelper
{
public:
	QuadricHelper() {}
	static void Init() {}
	static vcg::math::Quadric<double>& Qd(VCGVertex& v) { return TD()[v]; }
	static vcg::math::Quadric<double>& Qd(VCGVertex* v) { return TD()[*v]; }
	static VCGVertex::ScalarType W(VCGVertex* /*v*/) { return 1.0; }
	static VCGVertex::ScalarType W(VCGVertex& /*v*/) { return 1.0; }
	static void Merge(VCGVertex& /*v_dest*/, VCGVertex const& /*v_del*/) {}
	static QuadricTemp*& TDp() { static QuadricTemp* td; return td; }
	static QuadricTemp& TD() { return *TDp(); }
};

typedef vcg::tri::BasicVertexPair<VCGVertex> VertexPair;

class VCGTriEdgeCollapse : public vcg::tri::TriEdgeCollapseQuadric< VCGMesh, VertexPair, VCGTriEdgeCollapse, QuadricHelper> {
public:
	typedef  vcg::tri::TriEdgeCollapseQuadric<VCGMesh, VertexPair, VCGTriEdgeCollapse, QuadricHelper> TECQ;
	inline VCGTriEdgeCollapse(const VertexPair& p, int i, vcg::BaseParameterClass* pp) :TECQ(p, i, pp) {}
};

class VCGTriEdgeCollapseQTex : public vcg::tri::TriEdgeCollapseQuadricTex<VCGMesh, VertexPair, VCGTriEdgeCollapseQTex, vcg::tri::QuadricTexHelper<VCGMesh>> {
public:
	typedef  TriEdgeCollapseQuadricTex<VCGMesh, VertexPair, VCGTriEdgeCollapseQTex, vcg::tri::QuadricTexHelper<VCGMesh>> TECQ;
	inline VCGTriEdgeCollapseQTex(const VertexPair& p, int i, vcg::BaseParameterClass* pp) :TECQ(p, i, pp) {}
};

void Simplification(VCGMesh& mesh, int targetFaceNum, vcg::tri::TriEdgeCollapseQuadricParameter& params, float& resultError)
{
	vcg::math::Quadric<double> q;
	q.SetZero();
	QuadricTemp td(mesh.vert, q);
	QuadricHelper::TDp() = &td;


	vcg::LocalOptimization<VCGMesh> optimization(mesh, &params);
	optimization.Init<VCGTriEdgeCollapse >();

	optimization.SetTargetSimplices(targetFaceNum);
	optimization.SetTimeBudget(1.0);
	while (optimization.DoOptimization() && mesh.FN() > targetFaceNum)
	{
		resultError = std::max(optimization.currMetric, resultError);
	}

	optimization.Finalize<VCGTriEdgeCollapse>();

	resultError = std::max(optimization.currMetric, resultError);
}

void TexSimplification(VCGMesh& mesh, int targetFaceNum, const vcg::tri::TriEdgeCollapseQuadricParameter& params, float& resultError)
{
	vcg::tri::TriEdgeCollapseQuadricTexParameter texParams;
	texParams.SetDefaultParams();
	texParams.BoundaryWeight = params.BoundaryQuadricWeight;
	texParams.CosineThr = params.CosineThr;
	//texParams.ExtraTCoordWeight = ;
	texParams.NormalCheck = params.NormalCheck;
	texParams.NormalThrRad = params.NormalThrRad;
	texParams.OptimalPlacement = params.OptimalPlacement;
	texParams.PreserveBoundary = params.PreserveBoundary || params.FastPreserveBoundary;
	texParams.PreserveTopology = params.PreserveTopology;
	texParams.QuadricEpsilon = params.QuadricEpsilon;
	texParams.QualityThr = params.QualityThr;
	texParams.QualityQuadric = params.QualityQuadric;
	//texParams.SafeHeapUpdate = ;
	texParams.ScaleFactor = params.ScaleFactor;
	texParams.ScaleIndependent = params.ScaleIndependent;
	texParams.UseArea = params.UseArea;
	texParams.UseVertexWeight = params.UseVertexWeight;

	vcg::math::Quadric<double> q;
	q.SetZero();
	vcg::tri::QuadricTexHelper<VCGMesh>::QuadricTemp td3(mesh.vert, q);
	vcg::tri::QuadricTexHelper<VCGMesh>::TDp3() = &td3;

	std::vector<std::pair<vcg::TexCoord2<float>, vcg::Quadric5<double>>> qv;

	vcg::tri::QuadricTexHelper<VCGMesh>::Quadric5Temp td(mesh.vert, qv);
	vcg::tri::QuadricTexHelper<VCGMesh>::TDp() = &td;

	vcg::LocalOptimization<VCGMesh> optimization(mesh, &texParams);
	optimization.Init<VCGTriEdgeCollapseQTex>();

	optimization.SetTargetSimplices(targetFaceNum);
	optimization.SetTimeBudget(1.0);
	while (optimization.DoOptimization() && mesh.FN() > targetFaceNum)
	{
		resultError = std::max(optimization.currMetric, resultError);
	};

	optimization.Finalize<VCGTriEdgeCollapseQTex>();

	resultError = std::max(optimization.currMetric, resultError);
}

void Simplify(VCGMesh& mesh, int targetFaceNum, double qualityThreshold, bool lockVertices, bool texSimplification, float& resultError)
{
	vcg::tri::TriEdgeCollapseQuadricParameter params = {};
	params.QualityThr = qualityThreshold;
	params.FastPreserveBoundary = true && !lockVertices;
	params.PreserveBoundary = false;
	params.PreserveTopology = true;
	params.NormalCheck = false;
	params.OptimalPlacement = false;
	params.ScaleIndependent = true;
	params.UseArea = true;
	params.QualityCheck = true;

	vcg::tri::UpdateTopology<VCGMesh>::VertexFace(mesh);
	vcg::tri::UpdateFlags<VCGMesh>::FaceBorderFromVF(mesh);
	vcg::tri::UpdateNormal<VCGMesh>::PerFace(mesh);
	vcg::tri::UpdateNormal<VCGMesh>::NormalizePerFace(mesh);
	vcg::tri::Clean<VCGMesh>::RemoveDuplicateVertex(mesh);
	vcg::tri::Clean<VCGMesh>::RemoveDegenerateFace(mesh);
	vcg::tri::Clean<VCGMesh>::RemoveZeroAreaFace(mesh);

	float resSqError = 0;

	if (texSimplification)
	{
		TexSimplification(mesh, targetFaceNum, params, resSqError);
	}
	else
	{
		Simplification(mesh, targetFaceNum, params, resSqError);
	}

	resultError = std::sqrtf(resSqError);
}