#ifndef PARTICLEEFFECT_H_
#define PARTICLEEFFECT_H_

struct ParticleEffect {
	uint8_t effectID;
	uint8_t u1, v1, u2, v2;
	uint8_t redTint, greenTint, blueTint;
	uint8_t frameCount;
	uint8_t particleCount;
	uint8_t size;
	int32_t sizeVariation;
	uint16_t spread;
	int32_t speed;
	int32_t gravity;
	int32_t baseLifetime, lifetimeVariation;
	uint8_t collideFlags;
	uint8_t fullBright;
};

#endif // PARTICLEEFFECT_H_
