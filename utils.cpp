#include <ntifs.h>
#include <windef.h>
#include <ntimage.h>
#include "shared.h"
#include "utils.h"

/**
 * \brief Get base address of kernel module
 * \param moduleName Name of the module (ex. storport.sys)
 * \return Address of the module or null pointer if failed
 */
void Utils::SetSEED()
{
	LARGE_INTEGER time = { 0ull };
	KeQuerySystemTime(&time);

	SEED = time.LowPart;
}

PVOID Utils::GetModuleBase(const char* moduleName)
{
	PVOID address = nullptr;
	ULONG size = 0;
	
	auto status = ZwQuerySystemInformation(SystemModuleInformation, &size, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return nullptr;

	auto* moduleList = static_cast<PSYSTEM_MODULE_INFORMATION>(ExAllocatePoolWithTag(NonPagedPool, size, POOL_TAG));
	if (!moduleList)
		return nullptr;

	status = ZwQuerySystemInformation(SystemModuleInformation, moduleList, size, nullptr);
	if (!NT_SUCCESS(status))
		goto end;

	for (auto i = 0; i < moduleList->ulModuleCount; i++)
	{
		auto module = moduleList->Modules[i];
		if (strstr(module.ImageName, moduleName))
		{
			address = module.Base;
			break;
		}
	}
	
end:
	ExFreePool(moduleList);
	return address;
}


/**
 * \brief Checks if buffer at the location of base parameter
 * matches pattern and mask
 * \param base Address to check
 * \param pattern Byte pattern to match
 * \param mask Mask containing unknown bytes
 * \return 
 */
bool Utils::CheckMask(const char* base, const char* pattern, const char* mask)
{
	for (; *mask; ++base, ++pattern, ++mask) 
	{
		if ('x' == *mask && *base != *pattern) 
		{
			return false;
		}
	}

	return true;
}

/**
 * \brief Find byte pattern in given buffer
 * \param base Address to start searching in
 * \param length Maximum length
 * \param pattern Byte pattern to match
 * \param mask Mask containing unknown bytes
 * \return Pointer to matching memory
 */
PVOID Utils::FindPattern(PVOID base, int length, const char* pattern, const char* mask)
{
	length -= static_cast<int>(strlen(mask));
	for (auto i = 0; i <= length; ++i) 
	{
		const auto* data = static_cast<char*>(base);
		const auto* address = &data[i];
		if (CheckMask(address, pattern, mask))
			return PVOID(address);
	}

	return nullptr;
}

/**
 * \brief Find byte pattern in given module/image ".text" and "PAGE" sections
 * \param base Base address of the kernel module
 * \param pattern Byte pattern to match
 * \param mask Mask containing unknown bytes
 * \return Pointer to matching memory
 */
PVOID Utils::FindPatternImage(PVOID base, const char* pattern, const char* mask)
{
	PVOID match = nullptr;

	auto* headers = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<char*>(base) + static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
	auto* sections = IMAGE_FIRST_SECTION(headers);
	
	for (auto i = 0; i < headers->FileHeader.NumberOfSections; ++i) 
	{
		auto* section = &sections[i];
		if ('EGAP' == *reinterpret_cast<PINT>(section->Name) || memcmp(section->Name, ".text", 5) == 0) 
		{
			match = FindPattern(static_cast<char*>(base) + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
			if (match) 
				break;
		}
	}

	return match;
}

/**
 * \brief Generate pseudo-random text into given buffer
 * \param text Pointer to text
 * \param length Desired length
 */
void Utils::RandomText(char* text, const int length)
{
	if (!text || length < 0)
		return;

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	auto seed = KeQueryTimeIncrement();

	for (auto n = 0; n < length; n++)
	{
		auto key = RtlRandomEx(&seed) % static_cast<int>(sizeof(alphanum) - 1);
		text[n] = alphanum[key];
	}
}

void Utils::RandomBaseboard(char* text, const int length) {
	if (!text || length < 0)
		return;

	// valid hexidecimal characters
	static const char hexidecimal_characters[] =
		"0123456789ABCDEFabcdef";

	const auto get_upper_case_char = []()->char
	{
		auto seed = KeQueryTimeIncrement();
		char result = 0;

		while (true)
		{
			auto seed = KeQueryTimeIncrement();
			auto _rand = RtlRandomEx(&seed);
			auto index = _rand % static_cast<int>(sizeof(hexidecimal_characters) - 1);

			result = hexidecimal_characters[index];
			if (!isdigit(result) && isupper(result) && !islower(result))
				break;
		}

		return result;
	};

	const auto get_lower_case_char = []()->char
	{
		auto seed = KeQueryTimeIncrement();
		char result = 0;

		while (true)
		{
			auto seed = KeQueryTimeIncrement();
			auto _rand = RtlRandomEx(&seed);
			auto index = _rand % static_cast<int>(sizeof(hexidecimal_characters) - 1);

			result = hexidecimal_characters[index];

			if (!isdigit(result) && islower(result) && !isupper(result))
				break;
		}

		return result;
	};

	const auto get_number_char = []()->char
	{
		auto seed = KeQueryTimeIncrement();
		char result = 0;

		while (true)
		{
			auto seed = KeQueryTimeIncrement();
			auto _rand = RtlRandomEx(&seed);
			auto index = _rand % static_cast<int>(sizeof(hexidecimal_characters) - 1);

			result = hexidecimal_characters[index];

			if (isdigit(result))
				break;
		}

		return result;
	};

	auto seed = KeQueryTimeIncrement();
	for (auto _index = 0; _index < length; _index++)
	{
		auto _rand = RtlRandomEx(&seed);
		auto index = _rand % static_cast<int>(sizeof(hexidecimal_characters) - 1);

		if (text[_index] == ' ' ||
			text[_index] == '\0' ||
			text[_index] == '-' ||
			text[_index] == '_' ||
			text[_index] == '=' ||
			text[_index] == '+' ||
			text[_index] == '{' ||
			text[_index] == '}' ||
			text[_index] == '\'' ||
			text[_index] == '"' ||
			text[_index] == '[' ||
			text[_index] == ']' ||
			text[_index] == ':' ||
			text[_index] == ';' ||
			text[_index] == ',' ||
			text[_index] == '.' ||
			text[_index] == '/' ||
			text[_index] == '?' ||
			text[_index] == '~' ||
			text[_index] == '`' ||
			text[_index] == '(' ||
			text[_index] == ')')
			continue;

		bool digit = isdigit(text[_index]);
		bool _upper = isupper(text[_index]);
		bool _lower = islower(text[_index]);

		if (!digit)
		{
			if (_upper)
				text[_index] = get_number_char();
			if (_lower)
				text[_index] = get_number_char();
		}
		else
		{
			text[_index] = get_number_char();
		}

		text[0] = get_upper_case_char();
	}
}

void Utils::RandomChars(char* text, const int length)
{
	if (!text || length < 0)
		return;

	// valid hexidecimal characters
	static const char hexidecimal_characters[] =
		"0123456789ABCDEF";

	const auto get_upper_case_char = []()->char
	{
		char result = 0;

		while (true)
		{
			auto index = RtlRandomEx(&Utils::SEED) % static_cast<int>(sizeof(hexidecimal_characters));

			result = hexidecimal_characters[index];
			if (!isdigit(result) && isupper(result) && !islower(result))
				break;
		}

		return result;
	};

	const auto get_lower_case_char = []()->char
	{
		auto seed = KeQueryTimeIncrement();
		char result = 0;

		while (true)
		{
			auto index = RtlRandomEx(&Utils::SEED) % static_cast<int>(sizeof(hexidecimal_characters));

			result = hexidecimal_characters[index];
			if (!isdigit(result) && islower(result) && !isupper(result))
				break;
		}

		return result;
	};

	const auto get_number_char = []()->char
	{
		char result = 0;

		while (true)
		{
			auto index = RtlRandomEx(&Utils::SEED) % static_cast<int>(sizeof(hexidecimal_characters));

			result = hexidecimal_characters[index];
			if (isdigit(result))
				break;
		}

		return result;
	};

	for (auto i = 0; i < length; i++)
	{
		auto _rand = RtlRandomEx(&Utils::SEED);

		if (text[i] == ' '  || 
			text[i] == '\0' || 
			text[i] == '-'  ||
			text[i] == '_'  ||
			text[i] == '='  ||
			text[i] == '+'  || 
			text[i] == '{'  ||
			text[i] == '}'  ||
			text[i] == '\'' ||
			text[i] == '"'  ||
			text[i] == '['  || 
			text[i] == ']'  ||
			text[i] == ':'  ||
			text[i] == ';'  ||
			text[i] == ','  ||
			text[i] == '.'  ||
			text[i] == '/'  ||
			text[i] == '?'  ||
			text[i] == '~'  || 
			text[i] == '`'  ||
			text[i] == '('  ||
			text[i] == ')')
			continue;

		bool digit = isdigit(text[i]);
		bool _upper = isupper(text[i]);
		bool _lower = islower(text[i]);

		if (!digit)
		{
			if (_upper)
				text[i] = get_upper_case_char();
			if (_lower)
				text[i] = get_lower_case_char();
		}
		else
		{
			text[i] = get_number_char();
		}
	}
}

int Utils::StrLen(const char* str) 
{
	int len = 0;
	const char* p = str;
	while (*p && ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f'))) {
		len++;
		p++;
	}
	return len;
}

void Utils::CreateGUID(_GUID* guid)
{
	if (!guid)
		return;

	auto alloc1 = ExAllocatePoolWithTag(NonPagedPool, 4, 0);
	auto alloc2 = ExAllocatePoolWithTag(NonPagedPool, 4, 0);

	LARGE_INTEGER_TIME perf_counter; 
	KeQueryPerformanceCounter(&perf_counter.counter1); 
	KeQueryPerformanceCounter(&perf_counter.counter2);

	guid->Data1 = (size_t)alloc1 + ((size_t)alloc1 - (size_t)alloc2) ^ perf_counter.counter1.LowPart ^ perf_counter.counter2.HighPart;
	guid->Data2 = (unsigned short)alloc1;
	guid->Data3 = (unsigned short)alloc2;
	guid->Data4[0] = (char)alloc1;
	guid->Data4[1] = (char)alloc1;
	guid->Data4[2] = (char)alloc2;
	guid->Data4[3] = (char)alloc1;
	guid->Data4[4] = (char)alloc1;
	guid->Data4[5] = (char)alloc2;
	guid->Data4[6] = (char)alloc2;
	guid->Data4[7] = (char)alloc2;

	ExFreePoolWithTag(alloc1, 0);
	ExFreePoolWithTag(alloc2, 0);
}

void Utils::SpoofGUID(void* data)
{
	GUID spoofed_guid;
	Utils::CreateGUID(&spoofed_guid);
	memcpy(data, &spoofed_guid, sizeof(spoofed_guid));
}

unsigned long Utils::Random(unsigned long* seed)
{
	unsigned long s = *seed * 1103515245 + 12345;
	*seed = s;
	return (s / 65536) % 32768;
}

unsigned long Utils::Hash(unsigned char* buffer, unsigned long length)
{
	if (!length) {
		return 0;
	}

	unsigned long h = (*buffer ^ 0x4B9ACE2F) * 0x1000193;
	for (unsigned long i = 1; i < length; ++i) {
		h = (buffer[i] ^ h) * 0x1000193;
	}
	return h;
}

void Utils::SpoofBuffer(unsigned long seed, unsigned char* buffer, unsigned long length)
{
	seed ^= Hash(buffer, length);
	for (unsigned long i = 0; i < length; ++i) {
		buffer[i] ^= (unsigned char)Random(&seed);
	}
}

PWCHAR Utils::TrimGUID(PWCHAR guid, unsigned long max)
{
	unsigned long i = 0;
	PWCHAR start = guid;

	--max;
	for (; i < max && *start != L'{'; ++i, ++start);
	for (; i < max && guid[i++] != L'}';);

	guid[i] = 0;
	return start;
}
