/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : py_std.h
	Description		 : The python exporter for w_std structs
	Comment          :
*/

#ifdef __PYTHON__

#ifndef __PY_STD_H__
#define __PY_STD_H__

#include "python_exporter/w_boost_python_helper.h"

namespace pywolf
{
	static void py_std_export()
	{
		using namespace boost::python;
		//define W_RESULT enum
		enum_<W_RESULT>("W_RESULT")
			.value("W_PASSED", W_RESULT::W_PASSED)
			.value("W_FAILED", W_RESULT::W_FAILED)
			.value("W_INVALIDARG", W_RESULT::W_INVALIDARG)
			.value("W_OUTOFMEMORY", W_RESULT::W_OUTOFMEMORY)
            .value("W_INVALID_FILE_ATTRIBUTES", W_RESULT::W_INVALID_FILE_ATTRIBUTES)
			.export_values()
			;
	}
}

#endif//__PY_STD_H__

#endif//__PYTHON__
