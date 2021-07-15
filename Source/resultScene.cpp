#include "ResultScene.h"
#include "../Library//sceneManager.h"
#include "../Source/debugScreen.h"

ResultScene::ResultScene()
{
	hImage = LoadGraph("data\\texture\\Score.png");
}

ResultScene::~ResultScene()
{
}

void ResultScene::Update()
{
	//�V�[���̐؂�ւ�
	if (CheckHitKey(KEY_INPUT_T))
	{
		SceneManager::ChangeScene("TitleScene");//�^�C�g��
	}
	SceneBase::Update();
}

void ResultScene::Draw()
{
	DrawGraph(0, 0, hImage, 0);
	DrawFormatString(400, 400, GetColor(0, 0, 0),"Score:");//�X�R�A(����)
	SetFontSize(50);
	SceneBase::Draw();
}
