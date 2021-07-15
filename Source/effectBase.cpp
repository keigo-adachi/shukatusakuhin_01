#include "effectBase.h"
#include "EffekseerForDXLib.h"

EffectBase::EffectBase(SceneBase * _scene) : GameObject(_scene)
{
	HandleEffect = -1;
	PlayingHandleEffect = -1;
	Position = VGet(0, 0, 0);
	Rotation = VGet(0, 0, 0);
	Time = 0;
}

EffectBase::EffectBase(SceneBase * _scene,const char * _HandleEffect, VECTOR _Position, VECTOR _Rotation,float _size)
{
	HandleEffect = LoadEffekseerEffect(_HandleEffect,_size);
	PlayingHandleEffect = PlayEffekseer3DEffect(HandleEffect);
	Position = _Position;
	Rotation = _Rotation;
}

EffectBase::~EffectBase()
{
	DeleteEffekseerEffect(HandleEffect);
}

void EffectBase::Start()
{
}

void EffectBase::Update()
{
}

void EffectBase::Draw()
{
	DrawEffekseer3D_Draw(PlayingHandleEffect);
}
