#pragma once
#include "../Library/sceneBase.h"

/// <summary>
/// ���U���g�V�[��
/// 
/// ���U���g��\�����āA�L�[����������^�C�g���Ɉڍs����B
/// </summary>
class ResultScene : public SceneBase {
public:
	ResultScene();
	~ResultScene();
	void Update() override;
	void Draw() override;
private:
	int hImage;//���U���g�摜�̃n���h��
};
