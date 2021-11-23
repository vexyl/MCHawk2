#ifndef EXTENDEDPROTOCOL_H_
#define EXTENDEDPROTOCOL_H_

#include "IProtocol.hpp"
#include "Packet.hpp"
#include "../Event.hpp"
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

class ExtendedProtocol final : public IProtocol {
public:
	static const uint32_t kExtInfoSize          = 67;
	static const uint32_t kExtEntrySize         = 69;
	static const uint32_t kCustomBlocksSize     = 2;
	static const uint32_t kSetTextHotkeySize    = 134;
	static const uint32_t kExtAddPlayerNameSize = 196;
	static const uint32_t kExtAddEntity2Size    = 138;
	static const uint32_t kChangeModelSize      = 66;
	static const uint32_t kDefineBlockSize      = 80;
	static const uint32_t kPlayerClickSize      = 15;

	enum Opcodes { kExtInfo = 0x10, kExtEntry = 0x11, kCustomBlocks = 0x13, kSetTextHotkey = 0x15, kExtAddPlayerName = 0x16, kExtAddEntity2 = 0x21, kChangeModel = 0x1D, kDefineBlock = 0x23, kPlayerClick = 0x22 };

	enum BlockType {
		kCobblestoneSlab = 0x32,
		kRope,
		kSandstone,
		kSnow,
		kFire,
		kLightPinkWool,
		kForestGreenWool,
		kBrownWool,
		kDeepBlue,
		kTurquoise,
		kIce,
		kCeramicTile,
		kMagma,
		kPillar,
		kCrate,
		kStoneBrick
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
	
	// Client <-> Server
	class ExtInfoPacket final : public Packet {
	public:
		Utils::MCString appName;
		uint16_t extensionCount;
		
		ExtInfoPacket() : ExtInfoPacket(std::string(), 0) {}
		ExtInfoPacket(Utils::MCString appName, uint16_t extensionCount) :
			Packet(Opcodes::kExtInfo, kExtInfoSize),
			appName(appName),
			extensionCount(extensionCount)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadMCString(appName);
			reader.ReadUInt16(extensionCount);

			extensionCount = ntohs(extensionCount);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteMCString(appName);
				writer->WriteUInt16(htons(extensionCount));
			}

			return writer;
		}
	};
	
	// Client <-> Server
	class ExtEntryPacket final : public Packet {
	public:
		Utils::MCString extName;
		uint32_t version;
		
		ExtEntryPacket() : ExtEntryPacket(std::string(), 0) {}
		ExtEntryPacket(Utils::MCString extName, uint32_t version) :
			Packet(Opcodes::kExtEntry, kExtEntrySize),
			extName(extName),
			version(version)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadMCString(extName);
			reader.ReadUInt32(version);

			version = ntohl(version);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteMCString(extName);
				writer->WriteUInt32(htonl(version));
			}

			return writer;
		}
	};
	
	// Client <-> Server
	class CustomBlocksPacket final : public Packet {
	public:
		uint8_t supportLevel;

		CustomBlocksPacket() : CustomBlocksPacket(0) {}
		CustomBlocksPacket(uint8_t supportLevel) :
			Packet(Opcodes::kCustomBlocks, kCustomBlocksSize),
			supportLevel(supportLevel)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadUInt8(supportLevel);
		}

		virtual std::unique_ptr<Utils::BufferStream> Serialize() override
		{
			auto writer = std::make_unique<Utils::BufferStream>(m_packetSize);

			if (writer != nullptr) {
				writer->WriteUInt8(m_opcode);
				writer->WriteUInt8(supportLevel);
			}

			return writer;
		}
	};

	// Client <-> Server
	class SetTextHotkeyPacket final : public Packet {
	public:
		Utils::MCString label;
		Utils::MCString action;
		uint32_t keyCode;
		uint8_t keyMods;

		SetTextHotkeyPacket() : SetTextHotkeyPacket(std::string(), std::string(), 0, 0) {}
		SetTextHotkeyPacket(Utils::MCString label, Utils::MCString action, uint32_t keyCode, uint8_t keyMods) :
			Packet(Opcodes::kSetTextHotkey, kSetTextHotkeySize),
			label(label),
			action(action),
			keyCode(keyCode),
			keyMods(keyMods)
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
				writer->WriteMCString(label);
				writer->WriteMCString(action);
				writer->WriteUInt32(htonl(keyCode));
				writer->WriteUInt8(keyMods);
			}

			return writer;
		}
	};

	// Server -> Client
	class ExtAddPlayerNamePacket final : public Packet {
	public:
		uint16_t nameID;
		Utils::MCString playerName;
		Utils::MCString listName;
		Utils::MCString groupName;
		uint8_t groupRank;

		ExtAddPlayerNamePacket() : ExtAddPlayerNamePacket(0, std::string(), std::string(), std::string(), 0) {}
		ExtAddPlayerNamePacket(uint16_t nameID, Utils::MCString playerName, Utils::MCString listName, Utils::MCString groupName, uint8_t groupRank) :
			Packet(Opcodes::kExtAddPlayerName, kExtAddPlayerNameSize),
			nameID(nameID),
			playerName(playerName),
			listName(listName),
			groupName(groupName),
			groupRank(groupRank)
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
				writer->WriteUInt16(htons(nameID));
				writer->WriteMCString(playerName);
				writer->WriteMCString(listName);
				writer->WriteMCString(groupName);
				writer->WriteUInt8(groupRank);
			}

			return writer;
		}
	};

	// Server -> Client
	class ExtAddEntity2Packet final : public Packet {
	public:
		int8_t entityID;
		Utils::MCString inGameName;
		Utils::MCString skinName;
		int16_t spawnX, spawnY, spawnZ;
		uint8_t spawnYaw, spawnPitch;

		ExtAddEntity2Packet() : ExtAddEntity2Packet(0, std::string(), std::string(), 0, 0, 0, 0, 0) {}
		ExtAddEntity2Packet(int8_t entityID, Utils::MCString inGameName, Utils::MCString skinName, uint16_t spawnX, uint16_t spawnY, uint16_t spawnZ, uint8_t spawnYaw, uint8_t spawnPitch) :
			Packet(Opcodes::kExtAddEntity2, kExtAddEntity2Size),
			entityID(entityID),
			inGameName(inGameName),
			skinName(skinName),
			spawnX(spawnX), spawnY(spawnY), spawnZ(spawnZ),
			spawnYaw(spawnYaw), spawnPitch(spawnPitch)
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
				writer->WriteInt8(entityID);
				writer->WriteMCString(inGameName);
				writer->WriteMCString(skinName);
				writer->WriteInt16(htons(spawnX));
				writer->WriteInt16(htons(spawnY));
				writer->WriteInt16(htons(spawnZ));
				writer->WriteUInt8(spawnYaw);
				writer->WriteUInt8(spawnPitch);
			}

			return writer;
		}
	};

	// Server -> Client
	class ChangeModelPacket final : public Packet {
	public:
		int8_t entityID;
		Utils::MCString modelName;

		ChangeModelPacket() : ChangeModelPacket(0, std::string()) {}
		ChangeModelPacket(int8_t entityID, Utils::MCString modelName) :
			Packet(Opcodes::kChangeModel, kChangeModelSize),
			entityID(entityID),
			modelName(modelName)
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
				writer->WriteInt8(entityID);
				writer->WriteMCString(modelName);
			}

			return writer;
		}
	};

	// Server -> Client
	class DefineBlockPacket final : public Packet {
	public:
		uint8_t blockID;
		Utils::MCString name;
		uint8_t solidity;
		uint8_t movementSpeed;
		uint8_t topTextureID, sideTextureID, bottomTextureID;
		uint8_t transmitLight;
		uint8_t walkSound;
		uint8_t fullBright;
		uint8_t shape;
		uint8_t blockDraw;
		uint8_t fogDensity;
		uint8_t fogR, fogG, fogB;

		DefineBlockPacket() : DefineBlockPacket(0, std::string(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {}
		DefineBlockPacket(
			uint8_t blockID,
			Utils::MCString name,
			uint8_t solidity,
			uint8_t movementSpeed,
			uint8_t topTextureID, uint8_t sideTextureID, uint8_t bottomTextureID,
			uint8_t transmitLight,
			uint8_t walkSound,
			uint8_t fullBright,
			uint8_t shape,
			uint8_t blockDraw,
			uint8_t fogDensity,
			uint8_t fogR, uint8_t fogG, uint8_t fogB
		) :
			Packet(Opcodes::kDefineBlock, kDefineBlockSize),
			blockID(blockID),
			name(name),
			solidity(solidity),
			movementSpeed(movementSpeed),
			topTextureID(topTextureID), sideTextureID(sideTextureID), bottomTextureID(bottomTextureID),
			transmitLight(transmitLight),
			walkSound(walkSound),
			fullBright(fullBright),
			shape(shape),
			blockDraw(blockDraw),
			fogDensity(fogDensity),
			fogR(fogR), fogG(fogG), fogB(fogB)
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
				writer->WriteUInt8(blockID);
				writer->WriteMCString(name);
				writer->WriteUInt8(solidity);
				writer->WriteUInt8(movementSpeed);
				writer->WriteUInt8(topTextureID);
				writer->WriteUInt8(sideTextureID);
				writer->WriteUInt8(bottomTextureID);
				writer->WriteUInt8(transmitLight);
				writer->WriteUInt8(walkSound);
				writer->WriteUInt8(fullBright);
				writer->WriteUInt8(shape);
				writer->WriteUInt8(blockDraw);
				writer->WriteUInt8(fogDensity);
				writer->WriteUInt8(fogR);
				writer->WriteUInt8(fogG);
				writer->WriteUInt8(fogB);
			}

			return writer;
		}
	};

	// Client->Server
	class PlayerClickPacket final : public Packet {
	public:
		uint8_t button;
		uint8_t action;
		uint16_t yaw, pitch;
		int8_t targetEntityID;
		int16_t targetBlockX, targetBlockY, targetBlockZ;
		uint8_t targetBlockFace;

		PlayerClickPacket() : PlayerClickPacket(0, 0, 0, 0, 0, 0, 0, 0, 0) {}
		PlayerClickPacket(
			uint8_t button,
			uint8_t action,
			uint16_t yaw, uint16_t pitch,
			int8_t targetEntityID,
			int16_t targetBlockX, int16_t targetBlockY, int16_t targetBlockZ,
			uint8_t targetBlockFace
		) :
			Packet(Opcodes::kPlayerClick, kPlayerClickSize),
				button(button),
				action(action),
				yaw(yaw), pitch(pitch),
				targetEntityID(targetEntityID),
				targetBlockX(targetBlockX), targetBlockY(targetBlockY), targetBlockZ(targetBlockZ),
				targetBlockFace(targetBlockFace)
		{}

		virtual void Deserialize(Utils::BufferStream& reader) override
		{
			// Skip opcode because each packet knows its opcode
			reader.Skip(sizeof(m_opcode));

			reader.ReadUInt8(button);
			reader.ReadUInt8(action);
			reader.ReadUInt16(yaw);
			reader.ReadUInt16(pitch);
			reader.ReadInt8(targetEntityID);
			reader.ReadInt16(targetBlockX);
			reader.ReadInt16(targetBlockY);
			reader.ReadInt16(targetBlockZ);
			reader.ReadUInt8(targetBlockFace);
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

	static std::shared_ptr<ExtInfoPacket> MakeExtInfoPacket(Utils::MCString appName, uint16_t extensionCount);
	static std::shared_ptr<ExtEntryPacket> MakeExtEntryPacket(Utils::MCString extName, uint32_t version);
	static std::shared_ptr<CustomBlocksPacket> MakeCustomBlocksPacket(uint8_t supportedLevel);
	static std::shared_ptr<SetTextHotkeyPacket> MakeSetTextHotkeyPacket(Utils::MCString label, Utils::MCString action, uint32_t keyCode, uint8_t keyMods);
	static std::shared_ptr<ExtAddPlayerNamePacket> MakeExtAddPlayerNamePacket(uint16_t nameID, Utils::MCString playerName, Utils::MCString listName, Utils::MCString groupName, uint8_t groupRank);
	static std::shared_ptr<ExtAddEntity2Packet> MakeExtAddEntity2Packet(int8_t entityID, Utils::MCString inGameName, Utils::MCString skinName, int16_t spawnX, int16_t spawnY, int16_t spawnZ, uint8_t spawnYaw, uint8_t spawnPitch);
	static std::shared_ptr<ChangeModelPacket> MakeChangeModelPacket(int8_t entityID, Utils::MCString modelName);
	static std::shared_ptr<DefineBlockPacket> MakeDefineBlockPacket(
		uint8_t blockID,
		Utils::MCString name,
		uint8_t solidity,
		uint8_t movementSpeed,
		uint8_t topTextureID, uint8_t sideTextureID, uint8_t bottomTextureID,
		uint8_t transmitLight,
		uint8_t walkSound,
		uint8_t fullBright,
		uint8_t shape,
		uint8_t blockDraw,
		uint8_t fogDensity,
		uint8_t fogR, uint8_t fogG, uint8_t fogB
	);

	::Event<Client*, PlayerClickPacket> playerClickEvents;

	ExtendedProtocol();

	virtual ~ExtendedProtocol() {}

	ExtendedProtocol(const ExtendedProtocol&) = delete;
	ExtendedProtocol& operator=(const ExtendedProtocol&) = delete;

	virtual size_t GetPacketSize(uint8_t opcode) const override;

	virtual bool HandleOpcode(uint8_t opcode, Client* client, Utils::BufferStream& reader) const override;

private:
	std::map<Opcodes, OpcodeHandler> m_defaultPacketHandlers;

	void ExtInfoOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void ExtEntryOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void CustomBlocksOpcodeHandler(Client* client, Utils::BufferStream& reader);
	void PlayerClickOpcodeHandler(Client* client, Utils::BufferStream& reader);
};
} // namespace Net

#endif // EXTENDEDPROTOCOL_H_
