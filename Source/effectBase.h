#pragma once
#include "../Library/gameObject.h"
class EffectBase : public GameObject{
public:
	EffectBase(SceneBase* _scene);
	EffectBase(SceneBase* _scene,const char * _HandleEffect,VECTOR _Position,VECTOR _Rotation,float _Size);
	~EffectBase();
	virtual void Start()override;
	virtual void Update()override;
	virtual void Draw()override;
private:
	int HandleEffect;
	int PlayingHandleEffect;
	int Time;
	VECTOR Position;
	VECTOR Rotation;
	float Size;
};