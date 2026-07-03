#pragma once

#include <vector>

#include "json.hpp"
#include <v8.h>

#include "../helpers/v8.hpp"
#include "Impl.hpp"

namespace WorldSnapshotApi
{
constexpr int FieldsPerEntry = 6;
constexpr int VehiclePoolSize = 2000;

inline nlohmann::json BuildRowsJson(const float* values, int count)
{
	nlohmann::json arr = nlohmann::json::array();
	for (int i = 0; i < count; ++i)
	{
		const float* entry = values + (i * FieldsPerEntry);
		arr.push_back(nlohmann::json::array({ entry[0], entry[1], entry[2], entry[3], entry[4], entry[5] }));
	}

	nlohmann::json retJson;
	retJson["ret"] = arr;
	return retJson;
}

inline nlohmann::json CallWorldSnapshot(int (*capiFn)(float*, int), int maxEntries)
{
	std::vector<float> buffer(static_cast<size_t>(maxEntries) * FieldsPerEntry);
	const int count = capiFn(buffer.data(), maxEntries);
	return BuildRowsJson(buffer.data(), count);
}

#define REGISTER_WORLD_SNAPSHOT_API(group, name, capiMember, maxEntriesExpr)                                 \
	class OMP_NODE_API_##group##_##name                                                                         \
	{                                                                                                           \
	public:                                                                                                     \
		OMP_NODE_API_##group##_##name()                                                                         \
		{                                                                                                       \
			APIManager::Instance().Register(#group, #name, &OMP_NODE_API_##group##_##name::Call_internal);      \
		}                                                                                                       \
                                                                                                                \
		static void Call_internal(const v8::FunctionCallbackInfo<v8::Value>& info)                               \
		{                                                                                                       \
			v8::Isolate* isolate = info.GetIsolate();                                                           \
			V8_CHECK_ARGS_LEN(0);                                                                               \
			auto* ompapi = Runtime::Instance().GetOMPAPI();                                                     \
			const auto ret = WorldSnapshotApi::CallWorldSnapshot(ompapi->capiMember, (maxEntriesExpr));         \
			info.GetReturnValue().Set(helpers::JsonToV8(isolate, ret));                                         \
		}                                                                                                       \
	} OMP_NODE_API_##group##_##name##_instance;

} // namespace WorldSnapshotApi
