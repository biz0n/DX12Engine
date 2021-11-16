#include "imgui.h"

typedef int ImGuiLayoutType;                
typedef int ImGuiLayoutItemType;        // -> enum ImGuiLayoutItemType_    // Enum: Item or Spring

enum ImGuiLayoutItemType_
{
    ImGuiLayoutItemType_Item,
    ImGuiLayoutItemType_Spring
};

// sizeof() == 48
struct ImGuiLayoutItem;

struct ImGuiLayout;



namespace ImGui
{
    // Stack Layout
    static ImGuiLayout*     FindLayout(ImGuiID id, ImGuiLayoutType type);
    static ImGuiLayout*     CreateNewLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size);
    static void             BeginLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size, float align);
    static void             EndLayout(ImGuiLayoutType type);
    static void             PushLayout(ImGuiLayout* layout);
    static void             PopLayout(ImGuiLayout* layout);
    static void             BalanceLayoutSprings(ImGuiLayout& layout);
    static ImVec2           BalanceLayoutItemAlignment(ImGuiLayout& layout, ImGuiLayoutItem& item);
    static void             BalanceLayoutItemsAlignment(ImGuiLayout& layout);
    static void             BalanceChildLayouts(ImGuiLayout& layout);
    static ImVec2           CalculateLayoutSize(ImGuiLayout& layout, bool collapse_springs);
    static ImGuiLayoutItem* GenerateLayoutItem(ImGuiLayout& layout, ImGuiLayoutItemType type);
    static float            CalculateLayoutItemAlignmentOffset(ImGuiLayout& layout, ImGuiLayoutItem& item);
    static void             TranslateLayoutItem(ImGuiLayoutItem& item, const ImVec2& offset);
    static void             BeginLayoutItem(ImGuiLayout& layout);
    static void             EndLayoutItem(ImGuiLayout& layout);
    static void             AddLayoutSpring(ImGuiLayout& layout, float weight, float spacing);
    static void             SignedIndent(float indent);

    IMGUI_API void          BeginHorizontal(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void          BeginHorizontal(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void          BeginHorizontal(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);
    IMGUI_API void          EndHorizontal();
    IMGUI_API void          BeginVertical(const char* str_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void          BeginVertical(const void* ptr_id, const ImVec2& size = ImVec2(0, 0), float align = -1.0f);
    IMGUI_API void          BeginVertical(int id, const ImVec2& size = ImVec2(0, 0), float align = -1);
    IMGUI_API void          EndVertical();
    IMGUI_API void          Spring(float weight = 1.0f, float spacing = -1.0f);
    IMGUI_API void          SuspendLayout();
    IMGUI_API void          ResumeLayout();

}

