#ifndef _BLOCK_DEF_H
#define _BLOCK_DEF_H

#include <string>
#include <cstdint>

struct BlockDef {
	uint8_t blockID;
	std::string name;
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

	// BlockDefinitionsExt
	bool useBlockDefinitionsExt;
	uint8_t leftTextureID, rightTextureID, frontTextureID, backTextureID;
	uint8_t minX, minY, minZ;
	uint8_t maxX, maxY, maxZ;
};

#endif // _BLOCK_DEF_H
