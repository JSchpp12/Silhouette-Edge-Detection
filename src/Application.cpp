#include "Application.hpp"

using namespace star; 

Application::Application(star::StarScene& scene) : StarApplication(scene){}

void Application::Load()
{
    std::cout << "--Application Controls--" << std::endl;
    std::cout << "Draw Shadow Volume Edges: B" << std::endl; 
    std::cout << "Draw Normals: N" << std::endl; 
    std::cout << "Draw Edge Highlights On Model: M" << std::endl; 

    this->camera.setPosition(glm::vec3{ -3.0, 1.0f, -3.0f});
    auto camPosition = this->camera.getPosition();
    this->camera.setLookDirection(-camPosition);

    auto mediaDirectoryPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory);
    auto cubePath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "models/cube/cube.obj";
    auto lionPath = mediaDirectoryPath + "models/lion-statue/source/rapid.obj";
    auto airplanePath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "models/airplane/Airplane.obj";

    auto icoSphere = mediaDirectoryPath + "models/icoSphere/low_poly_icoSphere.obj"; 
    auto sphere = star::BasicObject::New(icoSphere); 
    auto& i_i = sphere->createInstance(); 
    i_i.setScale(glm::vec3{ 0.1, 0.1, 0.1});
    this->scene.add(std::move(sphere)); 
    this->sphereInstance = &i_i; 
    
    auto air = SilhouetteObj::New(airplanePath);
    auto& a_i = air->createInstance();
    a_i.setScale(glm::vec3{ 0.001, 0.001, 0.001 });
    a_i.setPosition(glm::vec3{ 0.0, 0.0, 0.0 });
    this->airplane = static_cast<SilhouetteObj*>(&this->scene.getObject(this->scene.add(std::move(air))));

    //auto lion = SilhouetteObj::New(lionPath); 
    //auto& lion_i = lion->createInstance(); 
    //lion_i.setScale(glm::vec3{ 0.04f, 0.04f, 0.04f });
    //lion_i.setPosition(glm::vec3{ 0.0, 0.0, 0.0 });
    //lion_i.rotateGlobal(star::Type::Axis::x, -90);
    //lion_i.moveRelative(glm::vec3{ 0.0, -1.0, 0.0 });
    //this->scene.add(std::move(lion)); 

    auto lightHandle = this->scene.add(std::make_unique<star::Light>(star::Type::Light::point, glm::vec3{ 0, 1, 5 }));
    this->light = &this->scene.getLight(lightHandle); 
}

void Application::Update()
{


}

void Application::onKeyPress(int key, int scancode, int mods)
{
    if (key == star::KEY::B) {
        this->airplane->drawSilhouetteEdge = !this->airplane->drawSilhouetteEdge;
    }
    if (key == star::KEY::N) {
        this->airplane->drawNormals = !this->airplane->drawNormals;
    }
    if (key == star::KEY::M) {
        this->airplane->drawShadowVolume = !this->airplane->drawShadowVolume; 
    }

}

void Application::onKeyRelease(int key, int scancode, int mods)
{
}

void Application::onMouseMovement(double xpos, double ypos)
{
}

void Application::onMouseButtonAction(int button, int action, int mods)
{
}

void Application::onScroll(double xoffset, double yoffset)
{
}

void Application::onWorldUpdate()
{
    float mvmtAmt = AMT * lastFrameTime.timeElapsedLastFrameSeconds();
    auto lightPosition = this->light->getPosition();

    if (lightIncrease) {
        if (lightPosition.z < this->BOUNDARY) {
            lightPosition.z += mvmtAmt;
            this->light->setPosition(lightPosition);
            this->sphereInstance->setPosition(lightPosition); 
        }
        else
            lightIncrease = false;
    }
    else {
        if (lightPosition.z > -this->BOUNDARY) {
            lightPosition.z -= mvmtAmt;
            this->light->setPosition(lightPosition);
            this->sphereInstance->setPosition(lightPosition); 
        }
        else
            lightIncrease = true;
    }

    lastFrameTime.updateLastFrameTime();
}
