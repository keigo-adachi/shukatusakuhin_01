#include "effect.h"
#include <assert.h>
#include "EffekseerForDXLib.h"


Effect::Effect(SceneBase * _scene) : EffectBase(_scene)
{
}

Effect::Effect(SceneBase*_scene,VECTOR _Position,VECTOR _Rotation)
	:EffectBase(_scene, "data\\model\\Effect\\ban.efkefc",_Position,_Rotation,1.0f)
{
	Position = _Position;
	Rotation = _Rotation;
}

Effect::~Effect()
{
	DeleteEffekseerEffect(EffectHandle);
}

void Effect::Start()
{
}

void Effect::Update()
{
	// 定期的にエフェクトを再生する
	if (Time % 60 == 0)
	{
		// エフェクトを再生する。
		PlayingEffectHandle = PlayEffekseer3DEffect(EffectHandle);
	}

	// 再生中のエフェクトを移動する。
	SetRotationPlayingEffekseer3DEffect(PlayingEffectHandle,Rotation.x,Rotation.y,Rotation.z);
	SetPosPlayingEffekseer3DEffect(PlayingEffectHandle,Position.x,Position.y,Position.z);
	// 時間を経過させる。
	Time++;
	if (Time > 60)
	{
		DestroyMe();
	}
}

void Effect::Draw()
{
	DrawEffekseer3D_Draw(PlayingEffectHandle);
}
