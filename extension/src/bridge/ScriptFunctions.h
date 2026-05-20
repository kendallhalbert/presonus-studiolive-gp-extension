#pragma once

#include "gigperformer/sdk/types.h"

namespace presonus::studiolive::gpext::bridge
{

int scriptFunctionCount();
ExternalAPI_GPScriptFunctionDefinition *scriptFunctions();

} // namespace presonus::studiolive::gpext::bridge
