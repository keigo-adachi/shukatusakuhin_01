#pragma once
#include "../Library/sceneBase.h"

/// <summary>
/// �v���C�V�[��
/// </summary>
class PlayScene : public SceneBase {
public:
	PlayScene();
	~PlayScene();
	void Update() override;
	void Draw() override;
};