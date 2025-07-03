#pragma once

struct PlayerState { // Determines which effects are on the player-- no effect at 0, max at 255;
	unsigned char s_Paralyzed;
	unsigned char s_Blinded;
	char s_SpeedUp; // negetive is a slowdown
	char s_Strength; // negetive is weakness
	char s_Magnetism; // Determines charge on the puck

	PlayerState() {
		s_Paralyzed = 0;
		s_Blinded = 0;
		s_SpeedUp = 0; 
		s_Strength = 0;
		s_Magnetism = 0;
	}

	PlayerState(PlayerState& other) {
		this->s_Blinded = other.s_Blinded;
		this->s_Magnetism = other.s_Magnetism;
		this->s_Paralyzed = other.s_Paralyzed;
		this->s_SpeedUp = other.s_SpeedUp;
		this->s_Strength = other.s_Strength;
	}
};

struct Location {
	short s_x, s_y;

	Location() {
		s_x = 0;
		s_y = 0;
	}

	Location(short x, short y) {
		s_x = x;
		s_y = y;
	}

	Location(Location& other) {
		this->s_x = other.s_x;
		this->s_y = other.s_y;
	}

};

struct PlayerSize {
	short s_x, s_y;

	PlayerSize() {
		s_x = 1;
		s_y = 1;
	}

	PlayerSize(short x, short y) {
		s_x = x;
		s_y = y;
	}

	PlayerSize(PlayerSize& other) {
		this->s_x = other.s_x;
		this->s_y = other.s_y;
	}
};

struct Player {

	char s_HealthPoints;

	PlayerState state;
	Location pos;
	PlayerSize size;

	Player() = delete;
	
	Player(char health, PlayerState someState, Location somePos, PlayerSize someSize) {
		s_HealthPoints = health;
		state = PlayerState(someState);
		size = PlayerSize(someSize);
		pos = Location(somePos);
	}

};

struct Ball {

	char s_radius;

	Location pos;

	Ball() {};

	Ball(char ballRadius, Location somePos) {
		s_radius = ballRadius;
		pos = Location(somePos);
	}
};