
namespace offsets
{
	// client.dll                                                                                                           Addr       Value to modify
	inline std::ptrdiff_t cvar_unknown = 0xC4149A + 0x2; // If you set any value here, it will override the sig scan, (e.x: 0xC66D2A + 0x2)

	namespace signatures
	{
		const std::vector<uint8_t> cvar_unknown = { /*TODO*/ };
	}
}
