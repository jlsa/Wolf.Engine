import sys, os

#get path of script
_script_path = os.path.realpath(__file__)
_script_dir = os.path.dirname(_script_path)
pyWolfPath = _script_dir

if sys.platform == "linux" or sys.platform == "linux2":
    print "Linux not tested yet"
elif sys.platform == "darwin":
    print "OS X not tested yet"
elif sys.platform == "win32":
    pyWolfPath =  pyWolfPath + "\\..\\..\\..\\..\\bin\\x64\\Debug\\Win32\\"

if pyWolfPath != "" and (not pyWolfPath in sys.path):
    sys.path.append(pyWolfPath)

import ctypes, threading, pyWolf
from PySide import QtGui, QtCore
from PySide.QtGui import *
from PySide.QtCore import *

screen_width = 800
screen_height = 600

class scene(QWidget):
    def __init__(self, pContentPath, pLogPath, pAppName, parent = None):
        super(scene, self).__init__(parent)
        self.__exiting = False
        self._game = pyWolf.framework.w_game(pContentPath, pLogPath, pAppName)
        self._game.set_pre_init_callback(self.pre_init)
        self._game.set_post_init_callback(self.post_init)
        self._game.set_load_callback(self.load)
        self._game.set_update_callback(self.update)
        self._game.set_pre_render_callback(self.pre_render)
        self._game.set_post_render_callback(self.post_render)
        self._gDevice = None
        self._viewport = pyWolf.graphics.w_viewport()
        self._viewport_scissor = pyWolf.graphics.w_viewport_scissor()
        self._draw_command_buffers = pyWolf.graphics.w_command_buffers()
        self._draw_render_pass = pyWolf.graphics.w_render_pass()
        self._draw_fence = pyWolf.graphics.w_fences()
        self._draw_semaphore = pyWolf.graphics.w_semaphore()

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#The following codes have been added for this project
#++++++++++++++++++++++++++++++++++++++++++++++++++++
        self._shader = pyWolf.graphics.w_shader()
        self._pipeline = pyWolf.graphics.w_pipeline()
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++

        _config = pyWolf.graphics.w_graphics_device_manager_configs()
        _config.debug_gpu = False
        self._game.set_graphics_device_manager_configs(_config)

    def pre_init(self):
        print "pre_init"

    def post_init(self):
        #get main graphics device
        self._gDevice = self._game.get_graphics_device(0)
        print self._gDevice.get_info()
        print "post_init"

    def load(self):
        #initialize viewport
        self._viewport.y = 0
        self._viewport.width = screen_width
        self._viewport.height = screen_height
        self._viewport.minDepth = 0
        self._viewport.maxDepth = 1
        
        #initialize scissor of viewport
        self._viewport_scissor.offset.x = 0
        self._viewport_scissor.offset.y = 0
        self._viewport_scissor.extent.width = screen_width
        self._viewport_scissor.extent.height = screen_height

        #load render pass which contains frame buffers
        _render_pass_attachments = []
        _output_window = self._gDevice.output_presentation_window
        for _iter in _output_window.swap_chain_image_views:
            # COLOR                                 #DEPTH
            _render_pass_attachments.append([_iter, _output_window.depth_buffer_image_view])

        _hr = self._draw_render_pass.load(self._gDevice, self._viewport, self._viewport_scissor, _render_pass_attachments)
        if _hr:
            print "Error on loading render pass"
            sys.exit(1)

        #create one semaphore for drawing
        _hr = self._draw_semaphore.initialize(self._gDevice)
        if _hr:
            print "Error on initializing semaphore"
            sys.exit(1)

        #create one fence for drawing
        _hr = self._draw_fence.initialize(self._gDevice, 1)
        if _hr:
            print "Error on initializing fence(s)"
            sys.exit(1)

        #create one fence for drawing
        number_of_swap_chains = self._gDevice.get_number_of_swap_chains()
        _hr = self._draw_command_buffers.load(self._gDevice, number_of_swap_chains, pyWolf.graphics.w_command_buffer_level.PRIMARY)
        if _hr:
            print "Error on initializing draw command buffer(s)"
            sys.exit(1)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#The following codes have been added for this project
#++++++++++++++++++++++++++++++++++++++++++++++++++++
        #loading vertex shader
        _content_path_dir = _script_dir + "/content/"
        _hr = self._shader.load(self._gDevice, _content_path_dir + "shaders/shader.vert.spv", pyWolf.graphics.w_shader_stage_flag_bits.VERTEX_SHADER, "main")
        if _hr:
            print "Error on loading vertex shader"
            sys.exit(1)

        #loading fragment shader
        _hr = self._shader.load(self._gDevice, _content_path_dir + "shaders/shader.frag.spv", pyWolf.graphics.w_shader_stage_flag_bits.FRAGMENT_SHADER, "main")
        if _hr: 
            print "Error on loading fragment shader"
            sys.exit(1)

         #create pipeline
        _vba = pyWolf.graphics.w_vertex_binding_attributes()
        _hr = self._pipeline.load(self._gDevice, _vba, pyWolf.graphics.w_primitive_topology.TRIANGLE_LIST, self._draw_render_pass, self._shader, [self._viewport], [ self._viewport_scissor ])
        if _hr:
            print "Error on creating pipeline"
            sys.exit(1)

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++
        _hr = self.build_command_buffers()
        if _hr:
            print "Error on building draw command buffer(s)"
            sys.exit(1)
        
        print "scene loaded successfully"

    def build_command_buffers(self):
        _hr = pyWolf.W_PASSED
        _size = self._draw_command_buffers.get_commands_size()
        for i in xrange(_size):
            _cmd = self._draw_command_buffers.get_command_at(i)
            _hr = self._draw_command_buffers.begin(i, pyWolf.graphics.w_command_buffer_usage_flag_bits.SIMULTANEOUS_USE_BIT)
            if _hr:
                print "Error on begining command buffer: " + str(i)
                break
            
            self._draw_render_pass.begin(i, _cmd, pyWolf.system.w_color.CORNFLOWER_BLUE(), 1.0, 0)
            
            #place your draw code
            self._pipeline.bind(_cmd)
            self._gDevice.draw(_cmd, 3, 1, 0, 0)

            self._draw_render_pass.end(_cmd)
            
            _hr = self._draw_command_buffers.end(i)
            if _hr:
                print "Error on ending command buffer: " + str(i)
                break

        return _hr

    def update(self, pGameTime):
        print "fps: " + str(pGameTime.get_frames_per_second())
    
    def pre_render(self, pGameTime):
        _output_window = self._gDevice.output_presentation_window
        _frame_index = _output_window.swap_chain_image_index

        _wait_dst_stage_mask = [ pyWolf.graphics.w_pipeline_stage_flag_bits.COLOR_ATTACHMENT_OUTPUT_BIT ]
        _wait_semaphores = [ _output_window.swap_chain_image_is_available_semaphore ]
        _signal_semaphores = [ _output_window.rendering_done_semaphore ]
        _cmd = self._draw_command_buffers.get_command_at(_frame_index)
        _cmd_buffers = [_cmd]       

        #reset draw fence
        self._draw_fence.reset()
        _hr = self._gDevice.submit(_cmd_buffers, self._gDevice.graphics_queue, _wait_dst_stage_mask, _wait_semaphores, _signal_semaphores, self._draw_fence)
        if _hr:
            print "Error on submit to graphics device"
            return 

        _hr = self._draw_fence.wait()
        if _hr:
            print "Error on waiting for draw fence"
            return 

    
    def post_render(self, pSuccessfullyRendered):
        if pSuccessfullyRendered == False:
            print "Rendered Unsuccessfully"
    
    def run(self):
        #run game
        _window_info = pyWolf.system.w_window_info()
        _window_info.width = self.width()
        _window_info.height = self.height()
        _window_info.v_sync_enable = False
        _window_info.is_full_screen = False
        _window_info.swap_chain_format = 44 # BGRA8Unorm in VULKAN 
        _window_info.cpu_access_swap_chain_buffer = False

        # get window handle
        pycobject_hwnd = self.winId()
        #convert window handle as HWND to unsigned integer pointer for c++
        ctypes.pythonapi.PyCObject_AsVoidPtr.restype  = ctypes.c_void_p
        ctypes.pythonapi.PyCObject_AsVoidPtr.argtypes = [ctypes.py_object]
        int_hwnd = ctypes.pythonapi.PyCObject_AsVoidPtr(pycobject_hwnd)
        _window_info.set_win_id(int_hwnd)
  
        #initialize game
        _map_info = (0, _window_info)
        while True:
            if self.__exiting:
                self.release()
                break
            self._game.run(_map_info)

        print "Game exited"

    def showEvent(self, event):
        #run in another thread
        threading.Thread(target=self.run).start()
        event.accept()

    def closeEvent(self, event):
        self.__exiting = True
        event.accept()

    def release(self):
        self._draw_fence.release()
        self._draw_fence = None
        
        self._draw_semaphore.release()
        self._draw_semaphore = None
        
        self._draw_command_buffers.release()
        self._draw_command_buffers = None

        self._draw_render_pass.release()
        self._draw_render_pass = None

#++++++++++++++++++++++++++++++++++++++++++++++++++++
#The following codes have been added for this project
#++++++++++++++++++++++++++++++++++++++++++++++++++++
        self._shader.release()
        self._shader = None

        self._pipeline.release()
        self._pipeline = None
#++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++

        self._game.release()
        self._game = None
        self._gDevice = None
        self._viewport = None
        self._viewport_scissor = None
        
        
if __name__ == '__main__':
    # Create a Qt application
    app = QApplication(sys.argv)
    scene = scene(pyWolfPath + "..\\..\\..\\..\\content\\",
                  pyWolfPath,
                  "py_02_shader")
    scene.resize(screen_width, screen_height)
    scene.setWindowTitle('Wolf.Engine')
    scene.show()

    sys.exit(app.exec_())
