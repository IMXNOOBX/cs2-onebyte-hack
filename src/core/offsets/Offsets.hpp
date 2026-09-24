
namespace offsets
{
	// client.dll                                                                                                           Addr       Value to modify
	inline std::ptrdiff_t cvar_unknown = 0; // A nonzero RVA overrides the signature scan.

	namespace signatures
	{
		const std::vector<uint8_t> cvar_unknown = {
			0x80, 0x78, 0x58, 0x00,               // cmp byte ptr [rax+58h],0
			0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,   // jnz (0x00 bytes are wildcards)
			0x33, 0xC9                          // xor ecx,ecx
		};
	}
}
