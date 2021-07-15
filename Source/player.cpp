#include "player.h"
#include "Field.h"
#include "debugScreen.h"
#include "enemy.h"
#include <assert.h>
#include <cmath>
#include <math.h>
#include <string>

namespace
{
	static const std::string MODEL_CHARACTER = "data\\model\\Player\\���������[�܂�_.pmx";
	static const std::string FRAMENAME_MOVE = "�S�Ă̐e";
	static const VECTOR POSITION = VGet(-50, 0, 0);	//�������W
	static const VECTOR ROTATION = VGet(0.0f, 270.0f * DX_PI_F / 180.0f, 0.0f);	//������]�l
	static const float ANGLE_YAW = 180.0f * DX_PI_F / 180.0f; //�J�����̏�����]�l�i���j
	static const float ANGLE_PITCH = 0.0f;					  //�J�����̏�����]�l�i�c�j
	static const float CAMERA_RADIUS = 80.0f;	// �L�����N�^�[����J�����܂ł̋����i���a�j
	static const float RADIUS = 5.0f;			// �G�𓮂��������蔻��̔��a
	static const float SPEED_WALK = 1.0f;		// �����̈ړ���
	static const float SPEED_RUN = 5.0f;		// ����̈ړ���
	static const float JUMP_POWER = 3.0f;		// �W�����v��
	static const float JUMP_DOWNPOWER = 0.1f;	// �W�����v�͂��������
	static const float SPEED_CAMERA = 5.0f * DX_PI_F / 180.0f;		// �J�����̉�]�X�s�[�h
	// �U���̓����蔻��̃A�^�b�`����t���[���̖��O
	static const std::string FRAMENAME_ATTACK[Player::ATTACK::MAX] =
	{
		"���ܐ�",
		"�����",
		"�E���",
		"����"
	};
	// �U���̓����蔻��̔��a
	static const float RADIUS_ATTACK[Player::ATTACK::MAX] =
	{
		2.0f,
		2.0f,
		2.0f,
		3.0f,
	};
	// ���̍U���̓_�E�����邩�ǂ���(true�Ń_�E������)
	static const bool DOWN_ATTACK[Player::ATTACK::MAX] =
	{
		false,
		false,
		true,
		true
	};
}

Player::Player(SceneBase * scene) : GameObject(scene)
{
	// ���f���̓ǂݍ���
	hModel = MV1LoadModel(MODEL_CHARACTER.c_str());
	assert(hModel > 0);

	// �A�j���[�V�����̐���
	animation = new Animation(hModel);
	animation->Start(ANIM_ID::NEUTRAL, true);

	// �A�j���[�V�����ňړ������Ă���t���[���̔ԍ�����������
	MoveAnimFrameIndex = MV1SearchFrame(hModel, FRAMENAME_MOVE.c_str());

	// �A�j���[�V�����ňړ������Ă���t���[���𖳌��ɂ���
	MV1SetFrameUserLocalMatrix(hModel, MoveAnimFrameIndex, MGetIdent());

	// �R�c���f���̍��W��������
	Position = POSITION;
	Rotation = ROTATION;

	//�J�����̉�]�l
	YawAngle = ANGLE_YAW;
	PitchAngle = ANGLE_PITCH;

	jumpPower = JUMP_POWER;
	isJump = false;

	// �U���̓����蔻��̃A�^�b�`����t���[���̔ԍ�������
	for (int i = 0; i < ATTACK::MAX; i++)
	{
		Collision[i].FrameIndex = MV1SearchFrame(hModel, FRAMENAME_ATTACK[i].c_str());
		Collision[i].Radius = RADIUS_ATTACK[i];
		Collision[i].DownAttack = DOWN_ATTACK[i];
		Collision[i].isEnable = false;
	}
}

Player::~Player()
{
	// ���f���̍폜
	if (hModel > 0)
		MV1DeleteModel(hModel);
}

void Player::Start()
{
	// �I�u�W�F�N�g�̓ǂݍ���
	enemy = GetScene()->FindGameObject<Enemy>();
	assert(enemy != nullptr);

	Cm = GetScene()->FindGameObject<Camera>();
	assert(Cm != nullptr);

	effectManager = GetScene()->FindGameObject<EffectManager>();
	assert(effectManager != nullptr);
}

void Player::Update()
{
	// �ړ��ʂ��v�Z���邽�ߍX�V����O�̍��W���i�[
	TempPosition = Position;
	// �n�ʂƂ̓����蔻��
	HitField();

	if (CheckHitKey(KEY_INPUT_SPACE))
	{
		
	}

	// �X�e�[�^�X�̍X�V
	switch (status)
	{
	case STATE_NEUTRAL:
	case STATE_MOVE:
		Move();
		break;
	case STATE_WALK:
		Walk();
		break;
	case STATE_RUN:
		Run();
		break;
	case STATE_JUMP:
		Jump();
		break;
	case STATE_COMBOATTACK:
		ComboAttack();
		break;
	case STATE_DASHATTACK:
		DashAttack();
		break;
	case STATE_JUMPCOMBOATTACK:
		JumpComboAttack();
		break;
	case STATE_DODGE:
		Dodge();
		break;
	case STATE_GUARD:
		Guard();
		break;
	case STATE_JUMPGUARD:
		JumpGuard();
		break;
	case STATE_WASATTACK:
		WasAttack();
		break;
	case STATE_DOWN:
		Down();
		break;
	}

	// �����蔻��̏���
	for (int i = 0; i < ATTACK::MAX; i++)
	{
		// ���̓����蔻�肪�L�����ǂ���
		if (Collision[i].isEnable == false)
		{
			continue;
		}
		//�����蔻��̍��W���X�V
		Collision[i].Position = MV1GetFramePosition(hModel, Collision[i].FrameIndex);
		// ����������true
		if (enemy->HitCollision(Collision[i].Position, Collision[i].Radius, Collision[i].DownAttack))
		{
			//�����蔻��𖳌���
			Collision[i].isEnable = false;
		}
	}

	// �ړ��ʂ��v�Z
	Vel = VSub(Position, TempPosition);

	// �G�����炷
	enemy->CollisionPlayer(Position, RADIUS, Vel);

	// �J�����̍X�V
	SetCamera();

	// �A�j���[�V�����̃A�b�v�f�[�g
	animation->Update();
}

void Player::Draw()
{
	MATRIX m;

	//// �ړ��̍s������
	MATRIX mTrans = MGetTranslate(Position);

	//// Y����]�̍s������
	MATRIX mRotY = MGetRotY(Rotation.y);

	//// ��]�s��ƕ��s�ړ��s��
	m = MMult(mRotY, mTrans);

	//// �L�����N�^�[���f���̍s��̐ݒ�
	MV1SetMatrix(hModel, m);

	// �L�����N�^�[���f���`��
	MV1DrawModel(hModel);

	//�����蔻��̕`��
	for (int i = 0; i < ATTACK::MAX; i++)
	{
		if (Collision[i].isEnable == true)
		{
			DrawSphere3D(Collision[i].Position, Collision[i].Radius, 40, 8, GetColor(255, 255, 0), FALSE);
		}
	}
}

void Player::Move()
{
	// ���s�����s
	if (CheckHitKey(KEY_INPUT_W) ||
		CheckHitKey(KEY_INPUT_A) ||
		CheckHitKey(KEY_INPUT_S) ||
		CheckHitKey(KEY_INPUT_D))
	{
		beforestatus = STATE_MOVE;
		status = STATE_WALK;
		return;
	}

	// �W�����v�����s
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		// �W�����v�A�j���[�V�������Đ�
		animation->Start(ANIM_ID::JUMP, true);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_MOVE;
		status = STATE_JUMP;
		return;
	}

	// �A���U�������s
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z);//�G�̕�������
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		Collision[ATTACK::NORMAL_1].isEnable = true;
		status = STATE_COMBOATTACK;
		return;
	}
	// ��������s
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}

	// �K�[�h�����s
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		animation->Start(ANIM_ID::GUARD, true);
		status = STATE_GUARD;
		return;
	}

	// �ҋ@�A�j���[�V�������Đ�
	animation->Start(ANIM_ID::NEUTRAL, true);
}

void Player::Walk()
{
	// �������삵�Ă��Ȃ���Αҋ@��Ԃɖ߂�
	if (CheckHitKey(KEY_INPUT_W) == 0 &&
		CheckHitKey(KEY_INPUT_A) == 0 &&
		CheckHitKey(KEY_INPUT_S) == 0 &&
		CheckHitKey(KEY_INPUT_D) == 0)
	{
		beforestatus = STATE_WALK;
		status = STATE_NEUTRAL;
		return;
	}

	// �L�[���������Ƃő���̏��������s
	if (CheckHitKey(KEY_INPUT_LSHIFT) != 0)
	{
		beforestatus = STATE_WALK;
		status = STATE_RUN;
		return;
	}

	// �����A�j���[�V�������Đ�
	animation->Start(ANIM_ID::WALK, true);

	// �J�����̌����ɍ��킹�ăL�����N�^�[�̌�����ς���
	SetCharacterRotationY();

	MATRIX rotY = MGetRotY(Rotation.y); // Y���̉�]�s������

	// �ړ��̏���
	VECTOR vel = VTransform(VGet(0, 0, -SPEED_WALK), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�

	// �W�����v�����s
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		// �W�����v�A�j���[�V�������Đ�
		animation->Start(ANIM_ID::JUMP, false);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_WALK;
		status = STATE_JUMP;
		return;
	}

	// �A���U�������s
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z);
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		Collision[ATTACK::NORMAL_1].isEnable = true;
		status = STATE_COMBOATTACK;
		return;
	}

	//��������s
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}


	// �K�[�h�����s
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		animation->Start(ANIM_ID::GUARD, false);
		status = STATE_GUARD;
		return;
	}
}

void Player::Run()
{
	// �L�[�������Ă��Ȃ���Ε����̏��������s
	if (CheckHitKey(KEY_INPUT_LSHIFT) == 0)
	{
		beforestatus = STATE_RUN;
		status = STATE_WALK;
	}

	// ����A�j���[�V�������Đ�
	animation->Start(ANIM_ID::RUN, true);

	// �J�����̌����ɍ��킹�ăL�����N�^�[�̌�����ς���
	SetCharacterRotationY();

	MATRIX rotY = MGetRotY(Rotation.y); //��]�s������

	// �ړ��̏���
	VECTOR vel = VTransform(VGet(0, 0, -SPEED_RUN), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�

	// �W�����v�����s
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		//�W�����v�A�j���[�V�������Đ�
		animation->Start(ANIM_ID::JUMP, false);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_RUN;
		status = STATE_JUMP;
	}

	// �_�b�V���U�������s
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z); //�G�̕�������
		animation->Start(ANIM_ID::TACKLE, false);
		beforestatus = STATE_DASHATTACK;
		Collision[ATTACK::TACKLE_1].isEnable = true;
		status = STATE_DASHATTACK;
		return;
	}

	//��������s
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}
}

void Player::Jump()
{
	// �n�ʂɂ����珈������߂�
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// ����
	// �W�����v�͂����炷
	jumpPower -= JUMP_DOWNPOWER;
	//���W�ɃW�����v�͂�^����
	Position.y += jumpPower;

	MATRIX rotY = MGetRotY(Rotation.y);

	// ���W���ړ�
	if (beforestatus == STATE_WALK)
	{
		VECTOR vel = VTransform(VGet(0, 0, -SPEED_WALK), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
		Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
	}
	if (beforestatus == STATE_RUN)
	{
		VECTOR vel = VTransform(VGet(0, 0, -SPEED_RUN), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
		Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
	}

	//�@�W�����v�A���U�������s
	// �A���U�������s
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		status = STATE_JUMPCOMBOATTACK;
		return;
	}

	// �K�[�h�����s
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		animation->Start(ANIM_ID::GUARD, false);
		status = STATE_JUMPGUARD;
		return;
	}
}

void Player::ComboAttack()
{
	MATRIX rotY = MGetRotY(Rotation.y);
	VECTOR vel = VTransform(VGet(0, 0, -1.0f), rotY);	//��]�s����g�����x�N�g���̕ϊ�

	// �A������ڂ��ōU�����ς��
	switch (ComboNum)
	{
	case ATTACK::NORMAL_1:
		effectManager->CreateEffect(MV1GetFramePosition(hModel,Collision[ATTACK::NORMAL_1].FrameIndex), Rotation, 1);
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 15.0f)
			{
				Collision[ATTACK::NORMAL_1].isEnable = false;
				Collision[ATTACK::NORMAL_2].isEnable = true;
				animation->Start(ANIM_ID::ATTACK_2, false);
				ComboNum = ATTACK::NORMAL_2;
			}
		}
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_1].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	case ATTACK::NORMAL_2:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 5.0f)
			{
				Collision[ATTACK::NORMAL_2].isEnable = false;
				Collision[ATTACK::NORMAL_3].isEnable = true;
				animation->Start(ANIM_ID::ATTACK_3, false);
				ComboNum = ATTACK::NORMAL_3;
			}
		}
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_2].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	case ATTACK::NORMAL_3:
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_3].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	}
}

void Player::DashAttack()
{
	if (animation->IsFinish() == true)
	{
		Collision[ATTACK::TACKLE_1].isEnable = false;
		status = STATE_NEUTRAL;
	}
	if (animation->GetFrameTime() < 5.0f || animation->GetFrameTime() > 20.0f)
	{
		return;
	}

	//�ړ��ʂ̕������Ă�������ɐi�߂�	
	MATRIX rotY = MGetRotY(Rotation.y);
	VECTOR vel = VTransform(VGet(0, 0, -3.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
}

void Player::JumpComboAttack()
{
	// �n�ʂɂ����珈������߂�
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// ����
	// �W�����v�͂����炷
	jumpPower -= JUMP_DOWNPOWER;
	Position.y += jumpPower;

	// �A������ڂ��ōU�����ς��
	switch (ComboNum)
	{
	case ATTACK::NORMAL_1:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 15.0f)
			{
				Collision[ATTACK::NORMAL_1].isEnable = false;
				Collision[ATTACK::NORMAL_2].isEnable = true;
				animation->Start(ANIM_ID::ATTACK_2, false);
				ComboNum = ATTACK::NORMAL_2;
			}
		}
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_1].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	case ATTACK::NORMAL_2:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 5.0f)
			{
				Collision[ATTACK::NORMAL_2].isEnable = false;
				Collision[ATTACK::NORMAL_3].isEnable = true;
				animation->Start(ANIM_ID::ATTACK_3, false);
				ComboNum = ATTACK::NORMAL_3;
			}
		}
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_2].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	case ATTACK::NORMAL_3:
		if (animation->IsFinish() == true)
		{
			Collision[ATTACK::NORMAL_3].isEnable = false;
			status = STATE_NEUTRAL;
		}
		break;
	}
}

void Player::Dodge()
{
	if (animation->GetFrameTime() > 5.0f)
	{
		status = STATE_NEUTRAL;
	}
	if (animation->GetFrameTime() > 3.0f)
	{
		return;
	}
	MATRIX rotY = MGetRotY(Rotation.y);
	VECTOR vel = VTransform(VGet(0, 0, -10.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
}

void Player::Guard()
{
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		return;
	}
	status = STATE_NEUTRAL;
}

void Player::JumpGuard()
{
	// �n�ʂɂ����珈������߂�
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// ����
	// �W�����v�͂����炷
	jumpPower -= JUMP_DOWNPOWER;
	Position.y += jumpPower;
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		return;
	}
	animation->Start(ANIM_ID::JUMP, true);
	status = STATE_JUMP;

}

void Player::WasAttack()
{
}

void Player::Down()
{
}

void Player::HitField()
{
	// �n�ʂƂ̓����蔻��ׂ̈�Ray���쐬
	VECTOR upper = VAdd(Position, VGet(0, 1000, 0));
	VECTOR lower = VAdd(Position, VGet(0, -1000, 0));

	// �X�e�[�W��ǂݍ���
	Field* field = GetScene()->FindGameObject<Field>();

	//�@�n�ʂƂ̓����蔻��
	if (field->CollisionLine(&hit, upper, lower))
	{
		//�@�W�����v���Ă��Ȃ���΍��W��n�ʂƍ��W�����킹��
		if (isJump == false)
		{
			Position = hit;
		}
	}
	else
	{
		// �����ɒn�ʂ�������΍~��
		jumpPower -= JUMP_DOWNPOWER;
		Position.y += jumpPower;
	}
}

void Player::SetCamera()
{
	// �J��������
	float radius = CAMERA_RADIUS; //���a
	// ��
	if (CheckHitKey(KEY_INPUT_RIGHT))YawAngle -= SPEED_CAMERA;
	if (CheckHitKey(KEY_INPUT_LEFT))YawAngle += SPEED_CAMERA;
	// �c
	if (CheckHitKey(KEY_INPUT_UP))PitchAngle += SPEED_CAMERA;
	if (CheckHitKey(KEY_INPUT_DOWN))PitchAngle -= SPEED_CAMERA;

	CPos.y = sinf(PitchAngle) * radius;	// y���̍��W���X�V
	float radius2 = cosf(PitchAngle) * radius;			// xz���Ŏg�����a���쐬
	CPos.x = cos(YawAngle) * radius2;		// x���̍��W���X�V
	CPos.z = sinf(YawAngle) * radius2;	// z���̍��W���X�V
	Cm->SetTarget(VSub(VGet(Position.x, Position.y + 30.0f, Position.z), CPos));		// �����ʒu��ݒ�
	Cm->SetPosition(VAdd(VGet(Position.x, Position.y + 30.0f, Position.z), CPos));			// �J�����̈ʒu��ݒ�
}

void Player::SetCharacterRotationY()
{
	// �J�����̌����ɍ��킹�ăL�����N�^�[�̌�����ς���
	if (CheckHitKey(KEY_INPUT_W) != 0)
	{
		// �O
		Rotation.y = atan2(CPos.x, CPos.z);
		if (CheckHitKey(KEY_INPUT_A) != 0)
		{
			// ���O
			Rotation.y = atan2(CPos.x, CPos.z) + 315.0f * DX_PI_F / 180.0f;
		}
		if (CheckHitKey(KEY_INPUT_D) != 0)
		{
			// �E�O
			Rotation.y = atan2(CPos.x, CPos.z) + 45.0f * DX_PI_F / 180.0f;
		}
	}
	else if (CheckHitKey(KEY_INPUT_S) != 0)
	{
		// ��
		Rotation.y = atan2(CPos.x, CPos.z) + 180.0f * DX_PI_F / 180.0f;
		if (CheckHitKey(KEY_INPUT_A) != 0)
		{
			// ����
			Rotation.y = atan2(CPos.x, CPos.z) + 225.0f * DX_PI_F / 180.0f;
		}
		if (CheckHitKey(KEY_INPUT_D) != 0)
		{
			// �E��
			Rotation.y = atan2(CPos.x, CPos.z) + 135.0f * DX_PI_F / 180.0f;
		}
	}
	else if (CheckHitKey(KEY_INPUT_A) != 0)
	{
		// ��
		Rotation.y = atan2(CPos.x, CPos.z) + 270.0f * DX_PI_F / 180.0f;
	}
	else if (CheckHitKey(KEY_INPUT_D) != 0)
	{
		// �E
		Rotation.y = atan2(CPos.x, CPos.z) + 90.0f * DX_PI_F / 180.0f;
	}
}
