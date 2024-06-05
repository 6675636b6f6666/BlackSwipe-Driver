#pragma once

namespace Utils
{
	inline static unsigned long SEED = 0;

	void SetSEED();
	PVOID GetModuleBase(const char* moduleName);
	bool CheckMask(const char* base, const char* pattern, const char* mask);
	PVOID FindPattern(PVOID base, int length, const char* pattern, const char* mask);
	PVOID FindPatternImage(PVOID base, const char* pattern, const char* mask);
	void RandomText(char* text, const int length);
	void RandomChars(char* text, const int length);
	void RandomBaseboard(char* text, const int length);
	int StrLen(const char* str);
	void CreateGUID(_GUID* guid);
	void SpoofGUID(void* data);

	unsigned long Random(unsigned long* seed);
	unsigned long Hash(unsigned char* buffer, unsigned long length);
	void SpoofBuffer(unsigned long seed, unsigned char* buffer, unsigned long length);
	PWCHAR TrimGUID(PWCHAR guid, unsigned long max);

	

	template<typename Type, int Size>
	inline void SpoofIdentifier(void* data, bool should_print = false)
	{
		if (should_print)
		{
		
		}

		Type fake_identifiers[Size];
		memcpy(&fake_identifiers, data, Size);

		Utils::RandomChars(reinterpret_cast<char*>(fake_identifiers), Size);
		memcpy((void*)data, &fake_identifiers, Size);

		if (should_print)
		{
		
		}
	}
}
