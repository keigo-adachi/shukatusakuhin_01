#include "playScene.h"
#include "../Library/sceneManager.h"
#include "DebugScreen.h"
#include "axis.h"
#include "player.h"
#include "enemy.h"
#include "field.h"
#include "camera.h"
#include "effectManager.h"
PlayScene::PlayScene()
{
	//CreateGameObject<Axis>();	//���p��\��
	CreateGameObject<Player>(); //�v���C���[�̐���
	CreateGameObject<Enemy>();	//�G�l�~�[�̐���
	CreateGameObject<Field>();	//�t�B�[���h�̐���
	CreateGameObject<EffectManager>();	
	Camera *pCam = CreateGameObject<Camera>(); //�J�����̐���
	SetDrawOrder(pCam, 1);
}

PlayScene::~PlayScene()
{
}

void PlayScene::Update()
{
	//�V�[���̐؂�ւ�
	if (CheckHitKey(KEY_INPUT_T))
	{
		SceneManager::ChangeScene("TitleScene");//�^�C�g��
	}
	if (CheckHitKey(KEY_INPUT_R)) 
	{
		SceneManager::ChangeScene("ResultScene");//���U���g
	}
	SceneBase::Update();
}

void PlayScene::Draw()
{
	SceneBase::Draw();
}
