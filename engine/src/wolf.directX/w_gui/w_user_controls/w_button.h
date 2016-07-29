/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_button.h
	Description		 : The button user control class.This class modified and improved based on Microsoft DXUT library https://github.com/Microsoft/DXUT
	Comment          :
*/

#ifndef __W_BUTTON_H__
#define __W_BUTTON_H__

#include "w_label.h"
#include "w_image.h"

#define W_GUI_EVENT_BUTTON_CLICKED  0x0101
#define W_GUI_NEAR_BUTTON_DEPTH		0.6f
#define W_GUI_FAR_BUTTON_DEPTH		0.8f

namespace wolf
{
	namespace gui
	{
		class w_button : public w_label
		{
		public:
			w_button(_In_opt_ w_widget* pParent = nullptr);
			virtual ~ w_button();

			virtual HRESULT			  on_initialize(const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice) override;
			virtual bool			  handle_keyboard(_In_ UINT pMsg, _In_ WPARAM pWParam, _In_ LPARAM pLParam) override;
			virtual bool			  handle_mouse(_In_ UINT pMsg, _In_ const POINT& pPoint, _In_ WPARAM pwParam, _In_ LPARAM pLParam) override;
			virtual void			  on_hotKey() override;
			virtual bool			  contains_point(_In_ const POINT& pt) override;
			virtual bool			  can_have_focus() override;
			virtual void			  render(const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice, _In_ float pElapsedTime) override;
									  
#pragma region Getters				  
				
			float					  get_effect_min_value() const						{ return this->effect_min_value; }
			float					  get_effect_max_value() const					    { return this->effect_max_value; }
			W_COLOR					  get_button_normal_color() const					{ return DirectX::DWORD_COLOR_TO_W_COLOR(this->button_normal_color); }
			W_COLOR					  get_button_pressed_color() const					{ return DirectX::DWORD_COLOR_TO_W_COLOR(this->button_pressed_color); }
			W_COLOR					  get_button_mouse_over_color() const				{ return DirectX::DWORD_COLOR_TO_W_COLOR(this->button_mouse_over_color); }
			W_COLOR					  get_button_focused_color() const					{ return DirectX::DWORD_COLOR_TO_W_COLOR(this->button_focused_color); }
									  
			const wchar_t*			  get_icon_path() const								{ return this->icon_path.c_str(); }
			DirectX::XMFLOAT2		  get_icon_offset( )const							{ return DirectX::XMFLOAT2((float)this->icon_offset_x , (float)this->icon_offset_y); }
			DirectX::XMFLOAT2		  get_icon_scale()const								{ return DirectX::XMFLOAT2(this->icon_scale_x, this->icon_scale_y); }
									  
#pragma endregion					  
									  
#pragma region Setters				  
									  
			void					  set_effect_min_max_range(float pMin, float pMax)	{ this->effect_min_value = pMin; this->effect_max_value = pMax; }
									  
			void					  set_button_color(W_COLOR pColor)					{ this->button_normal_color = RGBA_TO_DWORD_COLOR(pColor.r, pColor.g, pColor.b, pColor.a); }
			void					  set_button_pressed_color(W_COLOR pColor)			{ this->button_pressed_color = RGBA_TO_DWORD_COLOR(pColor.r, pColor.g, pColor.b, pColor.a); }
			void					  set_button_mouse_over_color(W_COLOR pColor)		{ this->button_mouse_over_color = RGBA_TO_DWORD_COLOR(pColor.r, pColor.g, pColor.b, pColor.a); }
			void					  set_button_focused_color(W_COLOR pColor)			{ this->button_focused_color = RGBA_TO_DWORD_COLOR(pColor.r, pColor.g, pColor.b, pColor.a); }
									  
			void					  set_icon_path(_In_z_ const wchar_t* pValue)		{ this->icon_path = std::wstring(pValue); }
			void					  set_icon_offset(int pX, int pY)					{ this->icon_offset_x = pX; this->icon_offset_y = pY; }
			void					  set_icon_scale(float pX, float pY)				{ this->icon_scale_x = pX; this->icon_scale_y = pY; }

#pragma endregion					  
									  
		protected:					  
			bool					  pressed;
			float					  effect_min_value;
			float					  effect_max_value;
			std::unique_ptr<w_image>  icon;
			DWORD					  button_normal_color;
			DWORD					  button_pressed_color;
			DWORD					  button_mouse_over_color;
			DWORD					  button_focused_color;
			std::wstring			  icon_path;
			int						  icon_offset_x;
			int						  icon_offset_y;
			float					  icon_scale_x;
			float					  icon_scale_y;

		private:
			typedef w_label _super;


		};
	}
}

#endif