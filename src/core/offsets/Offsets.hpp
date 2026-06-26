
namespace offsets
{
	// client.dll                                                                                                           Addr       Value to modify
	inline std::ptrdiff_t cvar_unknown = 0; //0xC4149A + 0x2; // If you set any value here, it will override the sig scan, (e.x: 0xC66D2A + 0x2)

	namespace signatures
	{
		const std::vector<uint8_t> cvar_unknown = { 
			0x48, 0x89, 0x5C, 0x24, 0x20,           // mov [rsp+20],rbx
			0x80, 0x38, 0x00,                       // cmp byte ptr [rax],01
			0x0F, 0x85, 0x93, 0x00, 0x00, 0x00      // jne
		};
	}
}
