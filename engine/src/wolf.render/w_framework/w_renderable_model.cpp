#include "w_render_pch.h"
#include "w_renderable_model.h"
#include <w_vertex_declaration.h>

using namespace wolf::graphics;
using namespace wolf::framework;
using namespace wolf::content_pipeline;

w_renderable_model::w_renderable_model(_In_ const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice,
    _In_ w_model* pModelData, _In_ bool pZUp) :
_gDevice(pGDevice),
_model_content(nullptr),
_z_up(pZUp)
{
	if (pModelData)
	{
        this->_model_content = new w_model();
		//copy memory
		std::memcpy((void*)this->_model_content, pModelData, sizeof(w_model));
	}
}

w_renderable_model::~w_renderable_model()
{
    release();
}

HRESULT w_renderable_model::load(_In_ const w_render_pass* pRenderPass, _In_ const std::string& pPipelineCacheName)
{
    if (this->_model_content == nullptr) return S_FALSE;

	//load meshes
	std::vector<w_model::w_mesh*> _model_meshes;
	this->_model_content->get_meshes(_model_meshes);
    
    auto _t = this->_model_content->get_transform();
    
    std::vector<w_model::w_instance_info> _model_instances;
    this->_model_content->get_instances(_model_instances);
    if (_model_instances.size())
    {
        _create_instances_buffer(_model_instances.data(),
            static_cast<UINT>(_model_instances.size() * sizeof(w_model::w_instance_info)));
    }

    HRESULT _hr;
	uint8_t _error = 0;
    for (size_t i = 0; i < _model_meshes.size(); ++i)
	{
        auto _model_mesh = _model_meshes[i];
        auto _mesh = new wolf::graphics::w_mesh();
        
        //prepare vertices
        std::vector<float> _v_data;
        for (size_t j = 0; j < _model_mesh->vertices.size(); ++j)
        {
            auto _pos = _model_mesh->vertices[j].position;
            auto _uv = _model_mesh->vertices[j].uv;
            
            _v_data.push_back(_pos[0]);
            _v_data.push_back(_pos[1]);
            _v_data.push_back(_pos[2]);

            _v_data.push_back(_uv[0]);
            _v_data.push_back(1 - _uv[1]);
        }
               
        //load mesh
        _hr = _mesh->load(this->_gDevice,
                   _v_data.data(),
                   static_cast<UINT>(_v_data.size()),
                   static_cast<UINT>(_v_data.size() * sizeof(float)),
                   _model_mesh->indices.data(),
                   static_cast<UINT>(_model_mesh->indices.size()),
                   pRenderPass,
                   pPipelineCacheName,
                   this->_model_content->get_instnaces_count() ? w_shader_type::BASIC_INSTANCE_SHADER : w_shader_type::BASIC_SHADER,
                   _z_up);

        _v_data.clear();
        if (_hr == S_FALSE)
        {
            _error += 1;
            logger.error("Could not create mesh #" + std::to_string(i) + 
                " for model: " + this->_model_content->get_name());
            continue;
        }
             
        this->_meshes.push_back(_mesh);
    }
    
    return _error > 0 ? S_FALSE : S_OK;
}

HRESULT w_renderable_model::_create_instances_buffer(_In_ const void* const pInstancedData, _In_ const UINT pInstancedSize)
{
    if (!pInstancedData || pInstancedSize == 0) return S_OK;
    
    HRESULT _hr;
    
    //create DRAM
    w_buffer _staging_buffer;
    
    _hr = _staging_buffer.load_as_staging(this->_gDevice, pInstancedSize);
    if(_hr == S_FALSE)
    {
        logger.error("Could not create staging instance buffer of model: " + this->_model_content->get_name());
        return _hr;
    }
    
    _hr = _staging_buffer.set_data(pInstancedData);
    if(_hr == S_FALSE)
    {
        logger.error("Setting staging instance buffer of model: " + this->_model_content->get_name());
        return _hr;
    }
    
    _hr = _staging_buffer.bind();
    if(_hr == S_FALSE)
    {
        logger.error("Could not bind to staging instance buffer of model: " + this->_model_content->get_name());
        return _hr;
    }
    
    
    // create VRAM buffers
    _hr = this->_instances_buffer.load(this->_gDevice,
                                      pInstancedSize,
                                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                                      
    if(_hr == S_FALSE)
    {
        logger.error("Could not create instance buffer of model: " + this->_model_content->get_name());
        return _hr;
    }
    
    _hr = this->_instances_buffer.bind();
    if(_hr == S_FALSE)
    {
        logger.error("Could not bind to instance buffer of model: " + this->_model_content->get_name());
        return _hr;
    }
    
    //create one command buffer
    auto _copy_command_buffer = new w_command_buffers();
    _copy_command_buffer->load(this->_gDevice, 1);
    
    _copy_command_buffer->begin(0);
    {
        auto _copy_cmd = _copy_command_buffer->get_command_at(0);
        
        VkBufferCopy _copy_region = {};
        
        _copy_region.size = pInstancedSize;
        vkCmdCopyBuffer(_copy_cmd,
                        _staging_buffer.get_handle(),
                        this->_instances_buffer.get_handle(),
                        1,
                        &_copy_region);
    }
    _copy_command_buffer->flush(0);
    
    SAFE_DELETE(_copy_command_buffer);
    
    _staging_buffer.release();
    
    return S_OK;
}

HRESULT w_renderable_model::update(_In_ const glm::mat4 pViewProjection)
{
    auto _transform = this->_model_content->get_transform();
    auto _translate = glm::translate(glm::mat4(1.0f),
        glm::vec3(_transform.position[0], _transform.position[1], _transform.position[2]));
    glm::mat4 _scale = glm::scale(glm::mat4x4(1.0f),
        glm::vec3(_transform.scale[0], _transform.scale[1], _transform.scale[2]));

    auto _world = _translate *
        glm::rotate(_transform.rotation[0], _z_up ? glm::vec3(-1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::rotate(_transform.rotation[1], glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(_transform.rotation[2], glm::vec3(0.0f, 0.0f, 1.0f)) *
        _scale;

    HRESULT _hr = S_OK;
    for (size_t i = 0; i < this->_meshes.size(); ++i)
    {
        this->_meshes[i]->set_world(_world);
        this->_meshes[i]->set_view_projection(pViewProjection);
        if (this->_meshes[i]->update() == S_FALSE)
        {
            _hr = S_FALSE;
        }
    }

    return _hr;
}

void w_renderable_model::render(_In_ const VkCommandBuffer& pCommandBuffer)
{
    auto _instance_count = get_instances_count();
    auto _instance_handle = this->_instances_buffer.get_handle();

    std::for_each(this->_meshes.begin(), this->_meshes.end(), [&pCommandBuffer, &_instance_handle, &_instance_count](_In_ w_mesh* pMesh)
    {
        pMesh->render(pCommandBuffer, _instance_handle, _instance_count);
    });
}

ULONG w_renderable_model::release()
{
	if (this->get_is_released()) return 0;

	return _super::release();
}
