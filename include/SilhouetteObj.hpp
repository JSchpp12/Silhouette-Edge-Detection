#pragma once 

#include "BasicObject.hpp"
#include "ConfigFile.hpp"
#include "BasicObject.hpp"
#include "GeometryHelpers.hpp"
#include "BasicObject.hpp"

#include <memory>
#include <string>

class SilhouetteObj : public star::StarObject {
public:
	static std::unique_ptr<SilhouetteObj> New(const std::string path);

	void cleanupRender(star::StarDevice& device) override; 

	std::unique_ptr<star::StarPipeline> buildPipeline(star::StarDevice& device,
		vk::Extent2D swapChainExtent, vk::PipelineLayout pipelineLayout, 
		vk::RenderPass renderPass) override;

	virtual void recordRenderPassCommands(star::StarCommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, 
		int swapChainIndexNum, uint32_t vb_start, 
		uint32_t ib_start) override;

protected:
	std::unique_ptr<star::StarGraphicsPipeline> geoPipeline; 


	bool isBumpyMaterial = false; 

	SilhouetteObj(const std::string path);

	void loadFromFile(const std::string path);

	// Inherited via StarObject
	std::unordered_map<star::Shader_Stage, star::StarShader> getShaders() override;

};