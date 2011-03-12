/// @file eliza_nacl.cc
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser than calls the CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///
/// When the browser encounters JavaScript that references your NaCl module, it
/// calls the GetInstanceObject() method on the object returned from
/// CreateInstance().  In this example, the returned object is a subclass of
/// ScriptableObject, which handles the scripting support.

#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <cstdio>
#include <string>
#include "eliza.h"

/// These are the method names as JavaScript sees them.  Add any methods for
/// your class here.
namespace eliza {
    class ElizaModule : public pp:Module() {
        public:
            ElizaModule() :pp::Module(){}
            virtual ~ElizaModule(){}
            virtual pp::Instance* CreateInstance(PP_Instance instance){
                return new Eliza(instance);
            }
    };
}  // namespace

namespace pp{
    Module* CreateModule(){
        return new eliza::ElizaModule();
    }
}