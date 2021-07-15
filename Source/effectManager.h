#pragma once
#include "../Library/gameObject.h"
#include <vector>
#include <assert.h>
#include "effectBase.h"
#include "effect.h"
class EffectManager :public GameObject {
public:
	EffectManager(SceneBase*scene) {}

	void CreateEffect(VECTOR _Position,VECTOR _Rotation,int type) {
		EffectBase *ef;
		switch (type)
		{
		case 1:
			ef = new Effect(GetScene(),_Position,_Rotation);
			break;
		default:
			assert(false);
			break;
		}
		effect.push_back(ef);
	}

	void Update()override {
		for (auto it = effect.begin(); it != effect.end();) {
			(*it)->Update();
			if ((*it)->IsDestroy())
			{
				it = effect.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
	void Draw()override {
		for (auto ef : effect) { 
			ef->Draw(); 
		}
	}
private:
	std::vector<EffectBase*>effect;
};