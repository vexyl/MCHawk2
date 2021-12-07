#ifndef CLASSICPROTOCOL_H_
#define CLASSICPROTOCOL_H_

#include "IProtocol.hpp"
#include "Packet.hpp"
#include "../Utils/MCString.hpp"

#include <map>
#include <memory>

#ifdef __linux__
#include <arpa/inet.h>
#elif _WIN32
#define NOMINMAX // bug with winsock redefining min
#include <winsock2.h>
#undef NOMINMAX
#endif

namespace Net {
class Client;

class ClassicProtocol final : public IProtocol {
public:
	static const uint8_t kVersion = 0x07;

	// Client->Server
	static const uint32_t kAuthenticationSize       = 131;
	static const uint32_t kSetBlockSize             = 9;
	static const uint32_t kPositionOrientationSize  = 10;
	static const uint32_t kMessageSize              = 66;
	static const uint32_t kOrientationSize          = 4;
	static const uint32_t kDespawnSize              = 2;
	// Server->Client
	static const uint32_t kServerIdentificationSize = 131;
	static const uint32_t kLevelInitializeSize      = 1;
	static const uint32_t kLevelDataChunkSize       = 1028;
	static const uint32_t kLevelFinalizeSize        = 7;
	static const uint32_t kSetBlock2Size            = 8;
	static const uint32_t kSpawnPlayerSize          = 74;
	static const uint32_t kDisconnectPlayerSize     = 65;
	static const uint32_t kUserTypeSize             = 2;

	enum ClientOpcodes { kAuthentication = 0x00, kSetBlock = 0x05, kPositionOrientation = 0x08, kOrientation = 0x0b, kDespawn = 0x0c, kMessage = 0x0d };
	enum ServerOpcodes { kServerIdentification, kLevelInitializePacket=0x02, kLevelDataChunkPacket=0x03, kLevelFinalizePacket=0x04, kSetBlock2=0x06, kSpawnPlayerPacket = 0x07, kDisconnectPlayerPacket = 0x0e, kUserTypePacket = 0x0f};

	enum BlockType {
		kAir,
		kStone,
		kGrass,
		kDirt,
		kCobblestone,
		kWoodPlanks,
		kSapling,
		kBedrock,
		kFlowingWater,
		kStationaryWater,
		kFlowingLava,
		kStationaryLava,
		kSand,
		kGravel,
		kGoldOre,
		kIronOre,
		kCoalOre,
		kWood,
		kLeaves,
		kSponge,
		kGlass,
		kRedCloth,
		kOrangeCloth,
		kYellowCloth,
		kLimeCloth,
		kGreenCloth,
		kAquaGreenCloth,
		kCyanCloth,
		kBlueCloth,
		kPurpleCloth,
		kIndigoCloth,
		kVioletCloth,
		kMagentaCloth,
		kPinkCloth,
		kBlackCloth,
		kGrayCloth,
		kWhiteCloth,
		kDandelion,
		kRose,
		kBrownMushroom,
		kRedMushroom,
		kGoldBlock,
		kIronBlock,
		kDoubleSlab,
		kSlab,
		kBricks,
		kTNT,
		kBookshelf,
		kMossStone,
		kObsidian
	};

	static std::map<uint8_t, std::string> blockTypes;

	virtual bool IsValidBlock(uint8_t type) const override
	{
		auto iter = blockTypes.find(type);
		if (iter != blockTypes.end())
			return true;
		return false;
	}

	virtual std::string GetBlockNameByType(uint8_t type) const override
	{
		std::string name;
		auto iter = blockTypes.find(type);
		if (iter != blockTypes.end())
			name = iter->second;
		return name;
	}

	// Client -> Server
	class AuthenticationPacket final : public Packet {
	public:
		uint8_t version;
		Utils::MCString name, key;
		uint8_t UNK0;

		AuthenticationPacket() : AuthenticationPacket(0, std::string(), std::string(), 0) {}
		AuthenticationPacket(uint8_t version, Utils::MCString name, Utils::MCString key, uint8_t UNK0) :
			Packet(ClientOpcodes::kAuthentication, kAuthenticationSize),
			version(version),
			name(name), key(key),
			UNK0(UNK0)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadUInt8(version);
			reader.ReadMCString(name);
			reader.ReadMCString(key);
			reader.ReadUInt8(UNK0);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteUInt8(version);
				writer->WriteMCString(name);
				writer->WriteMCString(key);
				writer->WriteUInt8(UNK0);
			}

			return writer;
		}
	};

	// Client -> Server
	class SetBlockPacket final : public Packet {
	public:
		int16_t x, y, z;
		uint8_t mode, type;

		SetBlockPacket() : SetBlockPacket(0, 0, 0, 0, 0) {}
		SetBlockPacket(int16_t x, int16_t y, int16_t z, uint8_t mode, uint8_t type) :
			Packet(ClientOpcodes::kSetBlock, kSetBlockSize),
			x(x), y(y), z(z),
			mode(mode), type(type)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadInt16(x);
			reader.ReadInt16(y);
			reader.ReadInt16(z);
			reader.ReadUInt8(mode);
			reader.ReadUInt8(type);

			x = ntohs(x);
			y = ntohs(y);
			z = ntohs(z);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt16(htons(x));
				writer->WriteInt16(htons(y));
				writer->WriteInt16(htons(z));
				writer->WriteUInt8(mode);
				writer->WriteUInt8(type);
			}

			return writer;
		}
	};

	// Client -> Server
	class PositionOrientationPacket final : public Packet {
	public:
		int8_t pid;
		int16_t x, y, z;
		uint8_t yaw, pitch;

		PositionOrientationPacket() : PositionOrientationPacket(0, 0, 0, 0, 0, 0) {}
		PositionOrientationPacket(int8_t pid, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch) :
			Packet(ClientOpcodes::kPositionOrientation, kPositionOrientationSize),
			pid(pid),
			x(x), y(y), z(z),
			yaw(yaw), pitch(pitch)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadInt8(pid);
			reader.ReadInt16(x);
			reader.ReadInt16(y);
			reader.ReadInt16(z);
			reader.ReadUInt8(yaw);
			reader.ReadUInt8(pitch);

			x = ntohs(x);
			y = ntohs(y);
			z = ntohs(z);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt8(pid);
				writer->WriteInt16(htons(x));
				writer->WriteInt16(htons(y));
				writer->WriteInt16(htons(z));
				writer->WriteUInt8(yaw);
				writer->WriteUInt8(pitch);
			}

			return writer;
		}
	};

	// Client -> Server
	class OrientationPacket final : public Packet {
	public:
		int8_t pid;
		uint8_t yaw, pitch;

		OrientationPacket() : OrientationPacket(0, 0, 0) {}
		OrientationPacket(int8_t pid, uint8_t yaw, uint8_t pitch) :
			Packet(ClientOpcodes::kOrientation, kOrientationSize),
			pid(pid),
			yaw(yaw), pitch(pitch)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt8(pid);
				writer->WriteUInt8(yaw);
				writer->WriteUInt8(pitch);
			}

			return writer;
		}
	};

	// Server -> Client
	class DespawnPacket final : public Packet {
	public:
		int8_t pid;

		DespawnPacket() : DespawnPacket(0) {}
		DespawnPacket(int8_t pid) :
			Packet(ClientOpcodes::kDespawn, kDespawnSize),
			pid(pid)

		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt8(pid);
			}

			return writer;
		}
	};

	// Server -> Client || Client -> Server
	class MessagePacket final : public Packet {
	public:
		uint8_t flag;
		Utils::MCString message;

		MessagePacket() : MessagePacket(0, std::string()) {}
		MessagePacket(uint8_t flag, Utils::MCString message) :
			Packet(ClientOpcodes::kMessage, kMessageSize),
			flag(flag),
			message(message)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadUInt8(flag);
			reader.ReadMCString(message);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteUInt8(flag);
				writer->WriteMCString(message);
			}

			return writer;
		}
	};

	// Server -> Client
	class ServerIdentificationPacket final : public Packet {
	public:
		uint8_t version;
		Utils::MCString name, motd;
		uint8_t userType;

		ServerIdentificationPacket() : ServerIdentificationPacket(0, std::string(), std::string(), 0) {}
		ServerIdentificationPacket(uint8_t version, Utils::MCString name, Utils::MCString motd, uint8_t userType) :
			Packet(ServerOpcodes::kServerIdentification, kServerIdentificationSize),
			version(version),
			name(name), motd(motd),
			userType(userType)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadUInt8(version);
			reader.ReadMCString(name);
			reader.ReadMCString(motd);
			reader.ReadUInt8(userType);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteUInt8(version);
				writer->WriteMCString(name);
				writer->WriteMCString(motd);
				writer->WriteUInt8(userType);
			}

			return writer;
		}
	};

	// Server -> Client
	class LevelInitializePacket final : public Packet {
	public:
		LevelInitializePacket() : Packet(ServerOpcodes::kLevelInitializePacket, kLevelInitializeSize) {}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
			}

			return writer;
		}
	};

	// Server -> Client
	class LevelDataChunkPacket final : public Packet {
	public:
		uint16_t chunkLength;
		uint8_t chunkData[1024];
		uint8_t percent;

		LevelDataChunkPacket() :
			Packet(ServerOpcodes::kLevelDataChunkPacket, kLevelDataChunkSize),
			chunkLength(0),
			percent(0)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt16(htons(chunkLength));
				writer->WriteArray1024(chunkData);
				writer->WriteUInt8(percent);
			}

			return writer;
		}
	};

	// Server -> Client
	class LevelFinalizePacket final : public Packet {
	public:
		int16_t x, y, z;

		LevelFinalizePacket() : LevelFinalizePacket(0, 0, 0) {}
		LevelFinalizePacket(int16_t x, int16_t y, int16_t z) :
			Packet(ServerOpcodes::kLevelFinalizePacket, kLevelFinalizeSize),
			x(x), y(y), z(z)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt16(htons(x));
				writer->WriteInt16(htons(y));
				writer->WriteInt16(htons(z));
			}

			return writer;
		}
	};

	// Server -> Client
	class SetBlock2Packet final : public Packet {
	public:
		int16_t x, y, z;
		uint8_t type;

		SetBlock2Packet() : SetBlock2Packet(0, 0, 0, 0) {}
		SetBlock2Packet(int16_t x, int16_t y, int16_t z, uint8_t type) :
			Packet(ServerOpcodes::kSetBlock2, kSetBlock2Size),
			x(x), y(y), z(z),
			type(type)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt16(htons(x));
				writer->WriteInt16(htons(y));
				writer->WriteInt16(htons(z));
				writer->WriteUInt8(type);
			}

			return writer;
		}
	};

	// Server -> Client
	class SpawnPlayerPacket final : public Packet {
	public:
		int8_t pid;
		Utils::MCString name;
		int16_t x, y, z;
		uint8_t yaw, pitch;

		SpawnPlayerPacket() : SpawnPlayerPacket(0, std::string(), 0, 0, 0, 0, 0) {}
		SpawnPlayerPacket(int8_t pid, Utils::MCString name, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch) :
			Packet(ServerOpcodes::kSpawnPlayerPacket, kSpawnPlayerSize),
			pid(pid),
			name(name),
			x(x), y(y), z(z),
			yaw(yaw), pitch(pitch)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteInt8(pid);
				writer->WriteMCString(name);
				writer->WriteInt16(htons(x));
				writer->WriteInt16(htons(y));
				writer->WriteInt16(htons(z));
				writer->WriteUInt8(yaw);
				writer->WriteUInt8(pitch);
			}

			return writer;
		}
	};

	// Server -> Client
	class UserTypePacket final : public Packet {
	public:
		uint8_t type;

		UserTypePacket() : UserTypePacket(0) {}
		UserTypePacket(uint8_t type) :
			Packet(ServerOpcodes::kUserTypePacket, kUserTypeSize),
			type(type)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteUInt8(type);
			}

			return writer;
		}
	};

	// Server -> Client
	class DisconnectPlayerPacket final : public Packet {
	public:
		Utils::MCString reason;

		DisconnectPlayerPacket() : DisconnectPlayerPacket(std::string()) {}
		DisconnectPlayerPacket(Utils::MCString reason) :
			Packet(ServerOpcodes::kDisconnectPlayerPacket, kDisconnectPlayerSize),
			reason(reason)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadMCString(reason);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteMCString(reason);
			}

			return writer;
		}
	};

	static std::shared_ptr<Packet> MakePositionOrientationPacket(int8_t pid, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch);
	static std::shared_ptr<Packet> MakeOrientationPacket(int8_t pid, uint8_t yaw, uint8_t pitch);
	static std::shared_ptr<Packet> MakeMessagePacket(uint8_t flag, Utils::MCString message);
	static std::shared_ptr<Packet> MakeDespawnPacket(int8_t pid);
	static std::shared_ptr<Packet> MakeServerIdentificationPacket(uint8_t version, Utils::MCString name, Utils::MCString motd, uint8_t userType);
	static std::shared_ptr<Packet> MakeSpawnPlayerPacket(int8_t pid, Utils::MCString name, int16_t x, int16_t y, int16_t z, uint8_t yaw, uint8_t pitch);
	static std::shared_ptr<Packet> MakeLevelInitializePacket();
	static std::shared_ptr<Packet> MakeLevelFinalizePacket(int16_t x, int16_t y, int16_t z);
	static std::shared_ptr<Packet> MakeSetBlock2Packet(int16_t x, int16_t y, int16_t z, uint8_t type);
	static std::shared_ptr<Packet> MakeUserTypePacket(uint8_t type);
	static std::shared_ptr<Packet> MakeDisconnectPlayerPacket(Utils::MCString reason);

	std::function<void(Client*, const AuthenticationPacket&)> onAuthenticationCallback;
	std::function<void(Client*, const SetBlockPacket&)> onSetBlockCallback;
	std::function<void(Client*, const PositionOrientationPacket&)> onPositionOrientationCallback;
	std::function<void(Client*, const MessagePacket&)> onMessageCallback;

	ClassicProtocol();

	virtual ~ClassicProtocol() {}

	ClassicProtocol(const ClassicProtocol&) = delete;
	ClassicProtocol& operator=(const ClassicProtocol&) = delete;

	virtual size_t GetPacketSize(uint8_t opcode) const override;
	virtual bool HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const override;
private:
	std::map<ClientOpcodes, OpcodeHandler> m_defaultPacketHandlers;

	void AuthenticationOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void SetBlockOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void PositionOrientationOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void MessageOpcodeHandler(Client* client, Utils::BufferStream& reader);
};
} // namespace Net

#endif // CLASSICPROTOCOL_H_
