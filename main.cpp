#include <iostream>
#include <vector>
#include <random>

// Include GLAD before GLFW
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <uuid.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "BiBuild.h"
#include "setup.h"
#include "components/LightComponent.h"
#include "core/RenderSystem.h"
#include "core/ResourceManager.h"
#include "core/TextGenerator.h"
#include "core/Time.h"
#include "ObjectScripts/BiplaneScript.h"
#include "ObjectScripts/CameraScript.h"
#include "ObjectScripts/FilmScript.h"
#include "ObjectScripts/OutlineScript.h"
#include "ObjectScripts/TextScript.h"
#include "ObjectScripts/TVScript.h"
#include "gui/Debug.h"
#include "ObjectScripts/SkyboxScript.h"

const int WIN_WIDTH  = 1280;
const int WIN_HEIGHT = 720;
const char* WIN_TITLE  = "PGR Project";

const int animFrameTimeMs = 150;
bool Debug = false;


void CheckInteraction() {
    static bool intChck = false;
    static bool debugChecked = false;

    if (holubiho::InputManager::IsActionActive("ToggleDebug")) {
        if (!debugChecked){
            Debug = !Debug;
            debugChecked = true;
        }

    } else {
        debugChecked = false;
    }

    if (holubiho::InputManager::GetMouseMode() != GLFW_CURSOR_NORMAL || Debug) return;

    if (holubiho::InputManager::IsActionActive("Interact")) {
        if (intChck) return;
        intChck = true;
        if (auto* obj = holubiho::InputManager::ObjectUnderMouse()) {
            obj->hasBeenInteracted = true;
            std::cout << obj->name<<std::endl;
        }
    }else {
        intChck = false;
    }

}





int main() {
    if (!glfwInit()) return -1;

    loadConfig("nature_settings.txt");
    // Initialize GLAD
    holubiho::SceneManager scene = holubiho::SceneManager();
    holubiho::RenderSystem::Initialize(WIN_WIDTH,WIN_HEIGHT, WIN_TITLE, scene.cameraObject);


    scene.CreateSkyBox(skyboxFaces, nightSkyboxFaces);
    // BiBuild::RenderSystem::SetFogTexture(scene.skybox->GetComponent<BiBuild::ModelComponent>()->mat->textures[0]);
    holubiho::ResourceManager::LoadShaderProgram("phong", "./shaders/vertex/base.vert", "./shaders/fragment/basePhong.frag");
    auto* vegetationShader = holubiho::ResourceManager::LoadShaderProgram("vegetationShader", "./shaders/vertex/vegetation.vert", "./shaders/fragment/base.frag");
    vegetationShader->AddInfo("windDir", &windDir, holubiho::UniformType::Vec3);
    vegetationShader->AddInfo("windStrength", &windStrength, holubiho::UniformType::Float);
    vegetationShader->AddInfo("windSpeed", &windSpeed, holubiho::UniformType::Float);
    vegetationShader->AddInfo("time", holubiho::Time::fCurrentTimePointer(), holubiho::UniformType::Float);

    auto* waterShader = holubiho::ResourceManager::LoadShaderProgram("waterShader", "./shaders/vertex/water.vert", "./shaders/fragment/base.frag");
    waterShader->AddInfo("moveStrength", &waveHeight, holubiho::UniformType::Float);
    waterShader->AddInfo("moveSpeed", &waveSpeed, holubiho::UniformType::Float);
    waterShader->AddInfo("time", holubiho::Time::fCurrentTimePointer(), holubiho::UniformType::Float);


    holubiho::TextGenerator::Init(font);
    SetupInputBindings();


    auto camera = SetupCamera(&scene, &Debug);
    auto cameraScript = camera->GetScript<holubiho::CameraScript>();
    auto cameraComp = camera->GetComponent<holubiho::CameraComponent>();



    auto outline = scene.CreateObject("Outline");
    outline->AddScript<holubiho::OutlineScript>();


    auto tv = SetupTv(&scene);
    cameraScript->SetTVObject(tv);


    auto sceneObjs = scene.CreateObject("sceneObjs");
    holubiho::ResourceManager::LoadModelsFromFile("resources/scene/Scene.obj", sceneObjs, true);
    auto children = sceneObjs->GetChildren();
    children[4]->GetComponent<holubiho::ModelComponent>()->mat->shader = waterShader;
    children[9]->GetComponent<holubiho::ModelComponent>()->mat->shader = vegetationShader;
    children[7]->GetComponent<holubiho::ModelComponent>()->mat->shader = vegetationShader;
    children[6]->GetComponent<holubiho::ModelComponent>()->mat->shader = vegetationShader;
    sceneObjs->transform->localScale = glm::vec3(10.0f);


    auto plane = SetupPlane(&scene, &Debug);
    cameraScript->SetPlaneObject(plane);




    auto sign = scene.CreateObject("sign");
    holubiho::ResourceManager::LoadModelsFromFile("resources/Sign.glb", sign, false );
    auto text = scene.CreateObject("text");
    text->hasClickableParts = false;
    std::string textContent = "TV usage:\n- Interact with the green button to \nplay/pause videos.\n- Use the red and orange buttons to \nswitch between videos.\n- Enjoy the show!";
    auto textScript = holubiho::TextGenerator::CreateText(text, textContent, 0.0007, {1.0f, 0, 0});
    sign->AddChild(text);
    text->transform->localPosition = glm::vec3(0.02, 2.429f, 1.135f);
    text->transform->localRotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0,1,0));
    sign->transform->localPosition = glm::vec3(-0.283f, -0.047f, -9.593f);
    sign->transform->localRotation = glm::angleAxis(glm::radians(124.0f), glm::vec3(0,1,0));
    sign->transform->localScale = glm::vec3(0.578f);

    auto boat = scene.CreateObject("boat");
    holubiho::ResourceManager::LoadModelsFromFile("resources/low_poly_boat.glb", boat, false);
    boat->transform->localPosition = glm::vec3(-10.123f, 0.069f, -9.469f);
    boat->transform->localRotation = glm::angleAxis(glm::radians(-70.0f), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians(15.0f), glm::vec3(1,0,0));
    boat->transform->localScale = glm::vec3(0.547f);

    auto stopDayNightCycle = scene.CreateObject("stopDayNightCycle");
    auto model = stopDayNightCycle->AddComponent<holubiho::ModelComponent>();
    model->mesh = holubiho::ResourceManager::GetMesh("resources/cube.glb");
    model->mat = holubiho::ResourceManager::CreateMaterial("NextButtonMat");
    auto skyboxScript = scene.skybox->GetScript<holubiho::SkyboxScript>();
    skyboxScript->SetButton(stopDayNightCycle);
    stopDayNightCycle->transform->localPosition = glm::vec3(0, 0, -17.0f);



    while (!glfwWindowShouldClose(holubiho::RenderSystem::GetGLFWWindow())) {
        holubiho::Time::UpdateTime();
        static bool keybiningsReloaded = false;
        if (holubiho::InputManager::IsActionActive("ReloadSettings")) {
             if (!keybiningsReloaded) {
                 SetupInputBindings();
                 LoadTVVideosTXT("tv_videos.txt", tv);
                 loadConfig("nature_settings.txt");
                keybiningsReloaded = true;
            }
        }else {
            keybiningsReloaded = false;
        }

        glfwPollEvents();
        CheckInteraction();
        scene.UpdateScene();

        holubiho::RenderSystem::UpdateAndDraw(
            scene,
            cameraComp->GetViewMat(),
            cameraComp->GetProjectionMat()
        );
        holubiho::RenderSystem::DrawIDs(scene,
            cameraComp->GetViewMat(),
            cameraComp->GetProjectionMat()
        );
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (Debug) DrawDebug(&scene);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(holubiho::RenderSystem::GetGLFWWindow());
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(holubiho::RenderSystem::GetGLFWWindow());
    glfwTerminate();
    return 0;
}


