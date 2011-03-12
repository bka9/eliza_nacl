#include "eliza.h"

#include <stdio.h>
#include <stdlib.h>
#include <ppapi/c/pp_errors.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>

namespace {
    bool IsError(int32_t result) {
        return ((PP_OK != result) && (PP_ERROR_WOULDBLOCK != result));
    }
    const char* const kRespondMethodId = "respond";
    const char* const kStartMethodId = "start";
}  // namespace

namespace eliza{
Eliza::Eliza(pp::Instance instance)
    : pp:Instance(instance),
      url_request_(this),
      url_loader_(this),
      cc_factory_(this) {
    url_request_.SetURL("eliza.txt");
    url_request_.SetMethod("GET");
}
Eliza::~Eliza(){}

pp::Var Eliza::GetInstanceObject() {
  ElizaScriptObject* script_object = new ElizaScriptObject(this);
  return pp::Var(this, script_object);
}

bool Eliza::Start() {
  pp::CompletionCallback cc = cc_factory_.NewCallback(&Eliza::OnOpen);
  int32_t res = url_loader_.Open(url_request_, cc);
  if (PP_ERROR_WOULDBLOCK != res)
    cc.Run(res);

  return !IsError(res);
}

void Eliza::OnOpen(int32_t result) {
  if (result < 0)
    ReportResultAndDie("eliza.txt", "pp::Eliza::Open() failed", false);
  else
    ReadBody();
}

void Eliza::OnRead(int32_t result) {
  if (result < 0) {
    ReportResultAndDie("eliza.txt",
                       "pp::Eliza::ReadResponseBody() result<0",
                       false);

  } else if (result != 0) {
    int32_t num_bytes = result < kBufferSize ? result : sizeof(buffer_);
    url_response_body_.reserve(url_response_body_.size() + num_bytes);
    url_response_body_.insert(url_response_body_.end(),
                              buffer_,
                              buffer_ + num_bytes);
    ReadBody();
  } else {  // result == 0, end of stream
    ReportResultAndDie(url_, url_response_body_, true);
  }
}

void Eliza::ReadBody() {
  // Reads the response body (asynchronous) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  // Look at <ppapi/c/dev/ppb_url_loader> for more details.
  pp::CompletionCallback cc = cc_factory_.NewCallback(&Eliza::OnRead);
  int32_t res = url_loader_.ReadResponseBody(buffer_,
                                             sizeof(buffer_),
                                             cc);
  if (PP_ERROR_WOULDBLOCK != res)
    cc.Run(res);
}

void Eliza::ReportResultAndDie(const std::string& fname,
                                       const std::string& text,
                                       bool success) {
  ReportResult(fname, text, success);
  delete this;
}

void Eliza::ReportResult(const std::string& fname,
                                 const std::string& text,
                                 bool success) {
  if (success)
    printf("Eliza::ReportResult(Ok).\n");
  else
    printf("Eliza::ReportResult(Err). %s\n", text.c_str());
  fflush(stdout);
  pp::Module* module = pp::Module::Get();
  if (NULL == module)
    return;

  pp::Instance* instance = module->InstanceForPPInstance(instance_id_);
  if (NULL == instance)
    return;

  pp::Var window = instance->GetWindowObject();
  // calls JavaScript function reportResult(url, result, success)
  // defined in geturl.html.
  pp::Var exception;
  window.Call("reportResult", fname, text, success, &exception);
}

bool Eliza::ElizaScriptObject::HasMethod(
    const pp::Var& method,
    pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  return (method_name == kRespondMethodId || method_name == kStartMethodId);
}

pp::Var Eliza::ElizaScriptObject::Call(
    const pp::Var& method,
    const std::vector<pp::Var>& args,
    pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  if (app_instance_ != NULL && method_name == kStartMethodId) {
    return app_instance_->Start();
  } else {
      return false;
  }
  return pp::Var();
}
}