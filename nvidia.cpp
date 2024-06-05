#include <ntifs.h>
#include <ntstrsafe.h>

#include "utils.h"
#include "shared.h"
#include "nvidia.h"


template<typename T>
T RandomBetween(T min_val, T max_val) {
	T rand_val;
	LARGE_INTEGER time;
	KeQuerySystemTime(&time);
	rand_val = RtlRandomEx(&time.LowPart) % (max_val - min_val + 1) + min_val;
	return rand_val;
}


namespace nvidia_ctx
{

	BOOLEAN init()
	{
		uint64_t nvlddmkm_base = (uint64_t)Utils::GetModuleBase("nvlddmkm.sys");
		if (!nvlddmkm_base) return FALSE;


		g_system = *(uint64_t*)(nvlddmkm_base + 0xB95D40); // 48 8B 05 ? ? ? ? 4C 8B F2 44 8B E9

		return (g_system != 0);
	}

	uint64_t gpu_data(uint32_t gpu_instance)
	{
		uint64_t gpu_sys = *(uint64_t*)(g_system + 0x1C0);
		uint64_t gpu_mgr = *(uint64_t*)(gpu_sys + 0x3CAD0);

		if (!gpu_mgr)
			return 0;

		gpu_sys += 0x3C8D0;

		uint64_t gpu_device{};

		while (1)
		{
			uint32_t found_instance = *(uint32_t*)(gpu_sys + 0x8);

			if (found_instance == gpu_instance)
			{
				uint64_t device = *(uint64_t*)gpu_sys;

				if (device != 0)
					gpu_device = device;

				break;
			}

			gpu_sys += 0x10;
		}

		return gpu_device;
	}

	uint64_t next_gpu(uint32_t device_mask, uint32_t* start_index)
	{
		if (*start_index >= NV_MAX_DEVICES)
			return 0;

		for (uint32_t i = *start_index; i < NV_MAX_DEVICES; ++i)
		{
			if (device_mask & (1U << i))
			{
				*start_index = i + 1;

				
				return gpu_data(i);
			}
		}

		*start_index = NV_MAX_DEVICES;

		return 0;
	}

	BOOLEAN change_uuid(uint64_t gpu_object)
	{
		if (*(uint8_t*)(gpu_object + 0x848))
		{
			uint8_t* uuid_data = (uint8_t*)(gpu_object + 0x849);

			// randomize your GPU UUID here
			uuid_data[0] = 0xE9;
			uuid_data[1] = 0x12;
			uuid_data[2] = 0x38;
			uuid_data[3] = 0xA0;
			uuid_data[4] = 0x7E;
			uuid_data[5] = 0x27;
			uuid_data[6] = 0xF8;
			uuid_data[7] = 0xC3;
			uuid_data[8] = 0xA5;
			uuid_data[9] = 0xB1;
			uuid_data[10] = 0x99;
			uuid_data[11] = 0xAB;
			uuid_data[12] = 0x7C;
			uuid_data[13] = 0x8C;
			uuid_data[14] = 0xCD;
			uuid_data[15] = 0xD2;
			/*uuid_data[0] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[1] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[2] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[3] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[4] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[5] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[6] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[7] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[8] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[9] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[10] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[11] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[12] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[13] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[14] = RandomBetween<uint8_t>(0x01, 0xFF);
			uuid_data[15] = RandomBetween<uint8_t>(0x01, 0xFF);*/

			return TRUE;
		}
		else
		{
			
		}

		return FALSE;
	}

	BOOLEAN spoof_gpu()
	{
		BOOLEAN status = FALSE;
		uint64_t gpu_sys = *(uint64_t*)(g_system + 0x1C0);

		if (!gpu_sys)
			return status;


		uint32_t gpu_index{},
			gpu_mask = *(uint32_t*)(gpu_sys + 0x754);

		// loops through all available GPU's (limited to NV_MAX_DEVICES)
		while (1)
		{
			uint64_t gpu_object = next_gpu(gpu_mask, &gpu_index);

			if (!gpu_object)
				break;


		
			if (change_uuid(gpu_object))
				status = TRUE;
			else
				status = FALSE;

		}

		return status;
	}
}
