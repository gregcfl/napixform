#include "Include/json.hpp"  // C++ Json Library
#include <napi.h>  // C++ Nodejs N-API library
#include <string>
#include <regex>
#include <vector>
#include "Include/str_stuff.h"

using json = nlohmann::json;
using std::vector;

//Forward Declare
Napi::Value WalkObject(nlohmann::basic_json<> const &input_template, nlohmann::basic_json<> const &input_obj, Napi::Object function_obj, Napi::Env const env);


// Create a javascript value from a json value type
Napi::Value get_napi_value_of(nlohmann::json::value_type const &item, Napi::Env const &env)
{
  if (item.is_string())
    return Napi::String::New(env, static_cast<std::string>(item));
  if (item.is_number())
    return Napi::Number::New(env, static_cast<double>(item));
  if (item.is_boolean())
    return Napi::Boolean::New(env, static_cast<bool>(item));
  return Napi::String::Value();
}

// Process a Json {ArrayOf: {}} object that defines how to iterate an array in
// the template
Napi::Value ProcessArray(nlohmann::basic_json<> const &item_definition,
                         nlohmann::basic_json<> const &input_obj,
                         nlohmann::basic_json<> base,
                         Napi::Object func_obj,
                         Napi::Env const &env)
{
  auto i_iterateOver = item_definition.find("IterateOver");
  auto i_arrayTemplate = item_definition.find("ArrayTemplate");
  vector<Napi::Value> individual_transformations;

  if ( i_iterateOver != item_definition.end() && i_arrayTemplate != item_definition.end()) {
    auto arr_iterateover = input_obj;
    auto array_pieces = tokenize(*i_iterateOver, '.');
    for ( auto piece : array_pieces ) {
      auto i = arr_iterateover.find(piece);
      if (i != arr_iterateover.end()) {
        arr_iterateover = *i;
      }
      else {
        break;
      }
    }
    if (arr_iterateover.is_array()) {
      auto return_array = Napi::Array::New(env);
      int x=0;
      for ( auto arr_item : arr_iterateover ) {
        auto an_item = WalkObject(*i_arrayTemplate, arr_item, func_obj, env);
        return_array.Set(x++, an_item);
      }
      return return_array;
    }
    return Napi::String::New(env, static_cast<std::string>(*i_iterateOver));
  }
  return Napi::Value::From(env, "process_array_failure");
}

// Process a text value, value will be either:
//  1. a hard-coded value
//  2. a template indicator to be deciphered into a result
Napi::Value ConvertText(nlohmann::basic_json<> const &item_value, nlohmann::basic_json<> const &input_obj, Napi::Env env)
{
  auto outer_obj = input_obj;
  auto item_pieces = tokenize(item_value, '.');
  static const std::regex match_end_as_square_brackets("^(.*)\\[([0-9]{1,6})\\]$");

  for (auto fieldName : item_pieces)
  {
    if (outer_obj.contains(fieldName))
    {
      auto item = outer_obj.find(fieldName);
      auto inside_value = *item;
      if (inside_value.is_object())
      {
        outer_obj = inside_value;
      }
      else
      {
        if (inside_value.is_null())
        {
          return Napi::Number::Value(env, nullptr);
        }
        return get_napi_value_of(inside_value, env);
      }
    }
    else
    {
      std::smatch matched;
      if (std::regex_search(fieldName, matched, match_end_as_square_brackets))
      {
        auto array_name = matched.str(1);
        int index = std::stoi(matched.str(2));
        auto item = outer_obj.find(array_name);

        if (item != outer_obj.end()) {
          auto an_array = *item;
          if (index >= 0 && static_cast<int>(an_array.size()) > index) {
            outer_obj = an_array[index];
            if ( !outer_obj.is_object() ) {
              return get_napi_value_of(outer_obj, env);
            }
          }
        }
      }
    }
  }
  return Napi::Value::From(env, item_value);
}

// Run a JS function defined in the JSon template {Funct {fnName, params}}
// which is passed into native on an in an included object
Napi::Value runJsFunction(nlohmann::basic_json<> const &item_definition,
                         nlohmann::basic_json<> const &input_obj,
                         nlohmann::basic_json<> base,
                         Napi::Object func_obj,
                         Napi::Env const &env)
{
  auto fn_name_iter = item_definition.find("fnName");
  if ( fn_name_iter != item_definition.end() )
  {
    auto fn_name = static_cast<std::string>(*fn_name_iter);
    auto fn_val = func_obj.Get(fn_name);
    if ( fn_val.IsFunction() )
    {
      auto args_vector = std::vector<napi_value>();
      auto params_iter = item_definition.find("params");
      if ( params_iter != item_definition.end())
      {
        auto paramlist = static_cast<std::string>(*params_iter);
        auto param_pieces = tokenize(paramlist, ',');
        for (auto param : param_pieces) {
          args_vector.push_back(ConvertText(param, input_obj, env));
        }
      }
      auto the_fn = Napi::Function(env, fn_val);
      auto rv = the_fn.Call(args_vector);
      return Napi::Value::From(env, rv);
    }
  }
  return Napi::String::From(env, "CUSTOM_FUNCTION_ERROR");
}

// Recursively walk the Json object template and return the javascript object
// created by it.
Napi::Value WalkObject(nlohmann::basic_json<> const &input_template, nlohmann::basic_json<> const &input_obj, Napi::Object const func_obj, Napi::Env const env)
{
  static nlohmann::basic_json<> base_ = nlohmann::json::object();
  base_ = base_.empty() ? input_obj : base_;

  auto new_obj = Napi::Object::New(env);
  for (auto &item : input_template.items())
  {
    if (item.value().is_object())
    {
      // Magic string: ArrayOf means return a processed array
      if (item.key() == "ArrayOf") {
        return ProcessArray(item.value(), input_obj, base_, func_obj, env);
      }
      else if (item.key() == "Funct") {
        return runJsFunction(item.value(), input_obj, base_, func_obj, env);
      }
      else {
        new_obj.Set(item.key(), WalkObject(item.value(), input_obj, func_obj, env));
      }
    }
    else if (item.value().is_string())
    {
      auto converted = ConvertText(item.value(), input_obj, env);
      if (!(converted.IsUndefined() || converted.IsNull()))
      {
        new_obj.Set(item.key(), converted);
      }
    }
    else if (item.value().is_boolean())
    {
      bool b_bool = static_cast<bool>(item.value());
      new_obj.Set(item.key(), b_bool);
    }
    else if (item.value().is_number())
    {
      if (item.value().is_number_integer())
      {
        int i_num = static_cast<int>(item.value());
        new_obj.Set(item.key(), i_num);
      }
      else
      {
        int f_num = static_cast<double>(item.value());
        new_obj.Set(item.key(), f_num);
      }
    }
  }
  return new_obj;
}

/* ********************************************************************** */
// Exported function:                                                     //
// Convertes the input data to the templated shape. Returns a new         //
// javascript value in the template shape with data from the input data   //
/* ********************************************************************** */
Napi::Value Convert(const Napi::CallbackInfo &info)
{
  Napi::Env calling_env = info.Env();

  Napi::Value return_object = Napi::Object::New(calling_env);
  int length = info.Length();
  Napi::Object functs;

  if (length !=2 && length !=3)
  {
    Napi::TypeError::New(calling_env, "ERROR:  Two or three parameters expected. A template and input data must be supplied.").ThrowAsJavaScriptException();
    return return_object;
  }
  if (!info[0].IsString())
  {
    Napi::TypeError::New(calling_env, "First parameter must be a JSON template string").ThrowAsJavaScriptException();
    return return_object;
  }
  if (!info[1].IsString())
  {
    Napi::TypeError::New(calling_env, "Second parameter must be a JSON object object").ThrowAsJavaScriptException();
    return return_object;
  }
  if ( length == 3 )
  {
    functs = info[2].As<Napi::Object>();
  }
  else
  {
    functs = Napi::Object::New(calling_env);
  }

  auto input_template = json::parse(static_cast<std::string>(info[0].As<Napi::String>()));
  auto input_obj = json::parse(static_cast<std::string>(info[1].As<Napi::String>()));

  return_object = WalkObject(input_template, input_obj, functs, calling_env);
  return return_object;
}

// Napi Wrapper to exported function
Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "Convert"), Napi::Function::New(env, Convert));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
