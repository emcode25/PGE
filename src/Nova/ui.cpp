#include <Nova/ui.hpp>

#define NOVA_HPP_NO_HEADERS
#include <Nova/nova.hpp>
#undef NOVA_HPP_NO_HEADERS

#include <Nova/components.hpp>
#include <Nova/const.hpp>
#include <Nova/objects.hpp>
#include <Nova/utils.hpp>

#include <imgui/imgui.h>
#include <nfd.h>

void Nova::EditorUI::MainMenu(GLFWwindow* window, const flecs::world& ecs, std::vector<flecs::entity>& objs)
{
    static bool texWindowOpen = false;
    bool quit = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            //TODO: Implement a save system
            ImGui::MenuItem("Save",  "Ctrl-S");
            ImGui::MenuItem("Save as...", "Ctrl-Shift-S");

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt-F4", &quit))
            {
                glfwSetWindowShouldClose(window, quit);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Import Texture..."))
            {
                texWindowOpen = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Object"))
        {
            if (ImGui::BeginMenu("New Object"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    auto cube = Nova::createCube(ecs);
                    cube.set_doc_name("Cube");

                    objs.push_back(cube);
                }

                if (ImGui::MenuItem("Camera"))
                {
                    auto cam = Nova::createCamera(ecs);
                    cam.set_doc_name("Camera");

                    objs.push_back(cam);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }

#ifndef UNIMPLEMENTED

    if(texWindowOpen)
    {
        ImGui::Begin("Import Texture", &texWindowOpen);

        //Path text box
        static nfdresult_t res;
        static char texPath[Nova::CONST::OBJECT_NAME_CHARACTER_LIMIT];
        ImGui::Text("Path:");
        ImGui::InputText("##path", texPath, IM_ARRAYSIZE(texPath));

        //Three dots box
        //If box pressed, get path
        if (ImGui::Button("Browse"))
        {
            char* path = NULL;
            res = NFD_OpenDialog(NULL, NULL, &path);
            strncpy(texPath, path, Nova::CONST::OBJECT_NAME_CHARACTER_LIMIT);
            free(path);
        }

        //Texture name box
        static char name[Nova::CONST::OBJECT_NAME_CHARACTER_LIMIT];
        ImGui::Text("Name:");
        ImGui::InputText("##new_tex_name", name, IM_ARRAYSIZE(name), ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank);
        //TODO: set name for texture

        //Type dropdown
        const char* texTypes[] = { "Diffuse", "Specular" };
        static Nova::TexType typeSelected = Nova::TexType::DIFFUSE;
        const char* comboPreview = texTypes[static_cast<int>(typeSelected)];

        if (ImGui::BeginCombo("Texture Type:", comboPreview))
        {
            for (int n = 0; n < IM_ARRAYSIZE(texTypes); n++)
            {
                const bool isSelected = (static_cast<int>(typeSelected) == n);
                if (ImGui::Selectable(texTypes[n], isSelected))
                {
                    typeSelected = static_cast<TexType>(n);
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        //Accept/close button
        if (ImGui::Button("Finish"))
        {
            if (res == NFD_OKAY)
            {
                std::shared_ptr<Nova::Texture> containerTexture = Nova::loadTexture(texPath, typeSelected);
                globalTextures.push_back(containerTexture);
            }

            texWindowOpen = false;
        }

        ImGui::End();
    }
#endif
}

void Nova::EditorUI::ShowObjectProperties(flecs::entity& obj)
{
    if(ImGui::Begin("Object Components"))
    {
        auto activeTransform = obj.get_ref<Nova::Component::Transform>();
        
        if(ImGui::CollapsingHeader("Transform"))
        {
            ImGui::DragFloat3("Position", &(activeTransform->position(0)), 0.05f);
            ImGui::DragFloat3("Rotation", &(activeTransform->rotation(0)), 1.0f);
            ImGui::DragFloat3("Scale", &(activeTransform->scale(0)), 0.1f);
        }

        if(obj.has<Nova::Component::Camera>() && ImGui::CollapsingHeader("Camera"))
        {
            auto camProps = obj.get_ref<Nova::Component::Camera>();

            ImGui::SliderFloat("FOV", &(camProps->fov), 1.0f, 45.0f, "%.1f");
            ImGui::DragFloat("Near", &(camProps->zNear), 0.1f, 0.0f, 0.0f, "%.1f");
            ImGui::DragFloat("Far", &(camProps->zFar), 0.1f, 0.0f, 0.0f, "%.1f");
        }
    }

    ImGui::End();
}

void Nova::EditorUI::ShowObjectList(std::vector<flecs::entity>& objs, flecs::entity& activeObj)
{
    ImGui::Begin("Object List", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
    
    static std::string name;
    if (ImGui::BeginListBox("Objects", ImGui::GetWindowContentRegionMax()))
    {
        for (auto e : objs)
        {
            const bool selected = (activeObj == e);
            name = e.doc_name();
            name += "##";
            name += e.raw_id();
            if (ImGui::Selectable(name.c_str(), selected))
            {
                //TODO: Allow multiple objects to be selected at once
                //This allows the object to be manipulated (one at a time)
                activeObj = e;
            }

            //Highlight selected object in list
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }

            //Rename Objects (right-click)
            if (ImGui::BeginPopupContextItem())
            {
                static char rename[Nova::CONST::OBJECT_NAME_CHARACTER_LIMIT];
                ImGui::Text("Rename:");
                ImGui::InputText("##rename", rename, IM_ARRAYSIZE(rename));

                if (ImGui::Button("Close"))
                {
                    //New name if the string is non-empty
                    if(strcmp(rename, "") != 0)
                    {
                        e.set_doc_name(rename);
                    }

                    std::strcpy(rename, "");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::EndListBox();
    }
    
    ImGui::End();
}