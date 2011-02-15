/* Copyright (c) 2011 Akamai Technologies, Inc. */


struct lua_State;

class Config
{
public:
	Config();
	int Init(const StringPiece& path) WARN_RET;

	list<struct sensor> sensors;
	RE2* filenames;
	time_t* last_modified;
	ScannerMode mode;

private:
	int load_config(const StringPiece& path) WARN_RET;
	int load_sensors() WARN_RET;

	lua_State* L;
	string filenames_str_;
	string last_modified_str_;
	string mode_str_;

	DISALLOW_COPY_AND_ASSIGN(Config);
};
