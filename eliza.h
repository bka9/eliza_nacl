#ifndef ELIZA_H
#define ELIZA_H

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/instance.h>
#include <string>

namespace eliza{
    class Eliza : public pp:Instance{
        public:
            explicit Eliza(pp::Instance instance_);
            virtual ~Eliza();
            virtual pp::Var GetInstanceObject();
            virtual bool Start();
        
        private:
            static const int kBufferSize = 4096;
            void OnOpen(int32_t result);
            void OnRead(int32_t result);
            void ReadBody();
            
            void ReportResult(const std::string& fname,
                const std::string& text,
                bool success);
              // Calls JS reportResult() and self-destroy (aka delete this)
            void ReportResultAndDie(const std::string& fname,
                const std::string& text,
                bool success);
                    
            PP_Instance instance_id_;
            pp::URLRequestInfo url_request_;
            pp::URLLoader url_loader_;  // URLLoader provides an API to download URLs.
            char buffer_[kBufferSize];  // buffer for pp::URLLoader::ReadResponseBody().
            std::string url_response_body_;  // Contains downloaded data.
            pp::CompletionCallbackFactory<Eliza> cc_factory_;
            
            class ElizaScriptObject : public pp::deprecated::ScriptableObject {
               public:
                explicit PiGeneratorScriptObject(Eliza* app_instance)
                    : pp::deprecated::ScriptableObject(),
                      app_instance_(app_instance) {}
                virtual ~ElizaScriptObject() {}
                // Return |true| if |method| is one of the exposed method names.
                virtual bool HasMethod(const pp::Var& method, pp::Var* exception);
            
                // Invoke the function associated with |method|.  The argument list passed
                // in via JavaScript is marshaled into a vector of pp::Vars.  None of the
                // functions in this example take arguments, so this vector is always empty.
                virtual pp::Var Call(const pp::Var& method,
                                     const std::vector<pp::Var>& args,
                                     pp::Var* exception);
               private:
                Eliza* app_instance_;  // weak reference.
              };
    };
}

#endif