#pragma once
#include "effectBase.h"

class Effect :public  EffectBase{
public:
	Effect(SceneBase* _scene);
	Effect(SceneBase*_scene, VECTOR _Position, VECTOR _Rotation);
	~Effect();
	void Start()override;
	void Update()override;
	void Draw()override;
private:
	int EffectHandle;
	VECTOR Position;
	VECTOR Rotation;
	int PlayingEffectHandle;
	int Time;
};
