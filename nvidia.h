#pragma once
#define NV_MAX_DEVICES 32

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

namespace nvidia_ctx {
	inline uint64_t g_system{};

	BOOLEAN init();
	uint64_t gpu_data(uint32_t gpu_instance);
	uint64_t next_gpu(uint32_t device_mask, uint32_t* start_index);
	BOOLEAN change_uuid(uint64_t gpu_object);
	BOOLEAN spoof_gpu();
}