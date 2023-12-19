#include "SilhouetteObj.hpp"

std::unique_ptr<SilhouetteObj> SilhouetteObj::New(const std::string path)
{
	return std::unique_ptr<SilhouetteObj>(new SilhouetteObj(path));
}

void SilhouetteObj::cleanupRender(star::StarDevice& device)
{
	this->geoPipeline.reset(); 

	this->star::StarObject::cleanupRender(device); 
}

std::unique_ptr<star::StarPipeline> SilhouetteObj::buildPipeline(star::StarDevice& device, vk::Extent2D swapChainExtent, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass)
{
	star::StarGraphicsPipeline::PipelineConfigSettings settings;
	star::StarGraphicsPipeline::defaultPipelineConfigInfo(settings, swapChainExtent, renderPass, pipelineLayout);
	settings.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleListWithAdjacency;

	////create geometry render
	//{
	//	std::string mediaPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory);

	//	std::string geomShaderPath = mediaPath + "/shaders/silhouetteEdgeExtrusion.geom";
	//	std::string vertShaderPath = mediaPath + "/shaders/silhouetteEdgeExtrusion.vert"; 
	//	std::string fragShaderPath = mediaPath + "/shaders/silhouetteEdgeExtrusion.frag"; 
	//	auto vertShader = star::StarShader(vertShaderPath, star::Shader_Stage::vertex); 
	//	auto fragShader = star::StarShader(fragShaderPath, star::Shader_Stage::fragment); 
	//	auto geomShader = star::StarShader(geomShaderPath, star::Shader_Stage::geometry);

	//	this->geoPipeline = std::make_unique<star::StarGraphicsPipeline>(device, settings, vertShader, fragShader, geomShader);
	//	this->geoPipeline->init(); 
	//}

	//normal create
	auto graphicsShaders = this->getShaders();

	auto newPipeline = std::make_unique<star::StarGraphicsPipeline>(device, settings, graphicsShaders.at(star::Shader_Stage::vertex), graphicsShaders.at(star::Shader_Stage::fragment));
	newPipeline->init();

	return std::move(newPipeline);
}

void SilhouetteObj::recordRenderPassCommands(star::StarCommandBuffer& commandBuffer, vk::PipelineLayout& pipelineLayout, int swapChainIndexNum, uint32_t vb_start, uint32_t ib_start)
{
	this->star::StarObject::recordRenderPassCommands(commandBuffer, pipelineLayout, swapChainIndexNum, vb_start, ib_start); 

	//for (auto& mesh : this->getMeshes()) {
	//	mesh->getMaterial().bind(commandBuffer, pipelineLayout, swapChainIndexNum); 

	//	commandBuffer.buffer(swapChainIndexNum).setLineWidth(1.0f);

	//	uint32_t instanceCount = static_cast<uint32_t>(this->instances.size());
	//	uint32_t vertexCount = star::CastHelpers::size_t_to_unsigned_int(mesh->getVertices().size());
	//	uint32_t indexCount = star::CastHelpers::size_t_to_unsigned_int(mesh->getIndices().size());
	//	commandBuffer.buffer(swapChainIndexNum).drawIndexed(indexCount, instanceCount, ib_start, 0, 0);

	//	vb_start += mesh->getVertices().size();
	//	ib_start += mesh->getIndices().size();
	//}
}

SilhouetteObj::SilhouetteObj(const std::string path)
{
	loadFromFile(path); 
}

std::unordered_map<star::Shader_Stage, star::StarShader> SilhouetteObj::getShaders()
{
	std::unordered_map<star::Shader_Stage, star::StarShader> shaders;

	if (isBumpyMaterial) {
		//load vertex shader
		std::string vertShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/bump.vert";
		shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::vertex, star::StarShader(vertShaderPath, star::Shader_Stage::vertex)));

		//load fragment shader
		std::string fragShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/bump.frag";
		shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::fragment, star::StarShader(fragShaderPath, star::Shader_Stage::fragment)));
	}
	else {
		//load vertex shader
		std::string vertShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/default.vert";
		shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::vertex, star::StarShader(vertShaderPath, star::Shader_Stage::vertex)));

		//load fragment shader
		std::string fragShaderPath = star::ConfigFile::getSetting(star::Config_Settings::mediadirectory) + "/shaders/default.frag";
		shaders.insert(std::pair<star::Shader_Stage, star::StarShader>(star::Shader_Stage::fragment, star::StarShader(fragShaderPath, star::Shader_Stage::fragment)));
	}

	return shaders;
}


void SilhouetteObj::loadFromFile(const std::string path)
{
	std::string texturePath = star::FileHelpers::GetBaseFileDirectory(path);
	std::string materialFile = star::FileHelpers::GetBaseFileDirectory(path);

	/* Load Object From File */
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), materialFile.c_str(), true)) {
		throw std::runtime_error(warn + err);
	}
	if (warn != "") {
		std::cout << "An error occurred while loading obj file" << std::endl;
		std::cout << warn << std::endl;
		std::cout << "Loading will continue..." << std::endl;
	}

	float maxVal = 0;
	size_t counter = 0;
	size_t shapeCounter = 0;
	size_t materialIndex = 0;
	size_t triangleCounter = 0;
	size_t threeCounter = 0;

	star::Vertex* currVertex = nullptr;
	star::Handle loadMaterialTexture;
	std::unique_ptr<std::vector<star::Vertex>> verticies;
	std::unique_ptr<std::vector<uint32_t>> indicies;
	std::unique_ptr<std::vector<std::pair<unsigned int, unsigned int>>> sortedIds;
	std::vector<std::unique_ptr<star::StarMesh>> meshes(shapes.size());
	tinyobj::material_t* currMaterial = nullptr;
	std::unique_ptr<star::StarMaterial> objectMaterial;
	std::vector<std::shared_ptr<star::StarMaterial>> preparedMaterials;

	if (materials.size() > 0) {
		//create needed materials
		for (size_t i = 0; i < materials.size(); i++) {
			currMaterial = &materials.at(i);
			std::unique_ptr<star::Texture> texture;
			std::unique_ptr<star::Texture> bumpMap;

			if (currMaterial->diffuse_texname != "") {
				texture = std::unique_ptr<star::Texture>(new star::Texture(texturePath + star::FileHelpers::GetFileNameWithExtension(currMaterial->diffuse_texname)));
			}

			//apply maps 
			if (currMaterial->bump_texname != "") {
				bumpMap = std::unique_ptr<star::Texture>(new star::Texture(texturePath + star::FileHelpers::GetFileNameWithExtension(currMaterial->bump_texname)));
			}

			//check if any material values are 0 - ambient is important
			if (currMaterial->ambient[0] == 0) {
				currMaterial->ambient[0] = 1.0;
				currMaterial->ambient[1] = 1.0;
				currMaterial->ambient[2] = 1.0;
			}

			if (bumpMap)
			{
				this->isBumpyMaterial = true;
				preparedMaterials.push_back(std::shared_ptr<star::BumpMaterial>(new star::BumpMaterial(glm::vec4(1.0),
					glm::vec4(1.0),
					glm::vec4(1.0),
					glm::vec4{
						currMaterial->diffuse[0],
						currMaterial->diffuse[1],
						currMaterial->diffuse[2],
						1.0f },
						glm::vec4{
							currMaterial->specular[0],
							currMaterial->specular[1],
							currMaterial->specular[2],
							1.0f },
							currMaterial->shininess,
							std::move(texture),
							std::move(bumpMap)
							)));
			}
			else {
				preparedMaterials.push_back(std::shared_ptr<star::TextureMaterial>(new star::TextureMaterial(glm::vec4(1.0),
					glm::vec4(1.0),
					glm::vec4(1.0),
					glm::vec4{
						currMaterial->diffuse[0],
						currMaterial->diffuse[1],
						currMaterial->diffuse[2],
						1.0f },
						glm::vec4{
							currMaterial->specular[0],
							currMaterial->specular[1],
							currMaterial->specular[2],
							1.0f },
							currMaterial->shininess,
							std::move(texture)
							)));
			}

		}

		//need to scale object so that it fits on screen
		//combine all attributes into a single object 
		int dIndex = 0;
		for (const auto& shape : shapes) {
			triangleCounter = 0;
			threeCounter = 0;
			counter = 0;

			//tinyobj ensures three verticies per triangle  -- assuming unique vertices 
			const std::vector<tinyobj::index_t>& indicies = shape.mesh.indices;
			auto fullInd = std::make_unique<std::vector<uint32_t>>(shape.mesh.indices.size());
			auto vertices = std::make_unique<std::vector<star::Vertex>>(shape.mesh.indices.size());
			size_t vertCounter = 0;
			for (size_t faceIndex = 0; faceIndex < shape.mesh.material_ids.size(); faceIndex++) {
				for (int i = 0; i < 3; i++) {
					dIndex = (3 * faceIndex) + i;
					auto newVertex = star::Vertex();
					newVertex.pos = glm::vec3{
						attrib.vertices[3 * indicies[dIndex].vertex_index + 0],
						attrib.vertices[3 * indicies[dIndex].vertex_index + 1],
						attrib.vertices[3 * indicies[dIndex].vertex_index + 2]
					};
					newVertex.color = glm::vec3{
						attrib.colors[3 * indicies[dIndex].vertex_index + 0],
						attrib.colors[3 * indicies[dIndex].vertex_index + 1],
						attrib.colors[3 * indicies[dIndex].vertex_index + 2],
					};

					if (attrib.normals.size() > 0) {
						newVertex.normal = {
							attrib.normals[3 * indicies[dIndex].normal_index + 0],
							attrib.normals[3 * indicies[dIndex].normal_index + 1],
							attrib.normals[3 * indicies[dIndex].normal_index + 2],
						};
					}

					newVertex.texCoord = {
						attrib.texcoords[2 * indicies[dIndex].texcoord_index + 0],
						1.0f - attrib.texcoords[2 * indicies[dIndex].texcoord_index + 1]
					};

					vertices->at(vertCounter) = newVertex;
					fullInd->at(vertCounter) = star::CastHelpers::size_t_to_unsigned_int(vertCounter);
					vertCounter++;
				};
			}

			if (shape.mesh.material_ids.at(shapeCounter) != -1) {
				//apply material from files to mesh -- will ignore passed values 
				meshes.at(shapeCounter) = std::unique_ptr<star::StarMesh>(new star::StarMesh(std::move(vertices), std::move(fullInd), preparedMaterials.at(shape.mesh.material_ids[0]), true));
			}
			shapeCounter++;
		}
	}

	this->meshes = std::move(meshes);
}