#include "enemy.h"
#include "Field.h"
#include "debugScreen.h"
#include <assert.h>
#include <cmath>
#include <math.h>
#include "player.h"

namespace
{
	const float JUMP_POWER = 3.0f; // �W�����v��
	const float JUMP_DOWNPOWER = 0.1f; // �W�����v�͂��������
}

Enemy::Enemy(SceneBase * scene) : GameObject(scene)
{
	// ���f���̓ǂݍ���
	hModel = MV1LoadModel("data\\model\\Enemy\\���������[�܂�_.pmx");
	assert(hModel > 0);

	// �A�j���[�V�����̐���
	animation = new Animation(hModel);
	animation->Start(ANIM_ID::NEUTRAL, true);

	// �A�j���[�V�����ňړ������Ă���t���[���̔ԍ�����������
	MoveAnimFrameIndex = MV1SearchFrame(hModel, "�S�Ă̐e");

	// �A�j���[�V�����ňړ������Ă���t���[���𖳌��ɂ���
	MV1SetFrameUserLocalMatrix(hModel, MoveAnimFrameIndex, MGetIdent());

	// �R�c���f���̍��W��������
	Position = VGet(50.0f, 0.0f, 0.0f);
	Rotation = VGet(0, 90 * DX_PI_F / 180.0f, 0);
	
	Angle = 0.0f;
	Angle2 = 0.0f;

	jumpPower = JUMP_POWER;

	isJump = false;

	TempPosition = Position;

	//�J�v�Z���R���W�����̏�����
	p1 = Position;
	p2 = VAdd(Position, VGet(0, 22.0f, 0));

	rad = 3.0f;
}

Enemy::~Enemy()
{
	// ���f���̍폜
	if (hModel > 0)
		MV1DeleteModel(hModel);
}

void Enemy::Start()
{
}

void Enemy::Update()
{
	// �n�ʂƂ̓����蔻��
	HitField();

	//// �X�e�[�^�X�̍X�V
	switch (status)
	{
	case STATE_NEUTRAL:
	case STATE_MOVE:
		Move();
		break;
	case STATE_WASATTACK:
		WasAttack();
		break;
	case STATE_DOWN:
		Down();
		break;
	case STATE_DOWN_LOOP:
		DownLoop();
		break;
	case STATE_STANDUP:
		StandUp();
		break;
	}

	// �A�j���[�V�����̃A�b�v�f�[�g
	animation->Update();

	p1 = Position;
	p2 = VAdd(Position, VGet(0, 22.0f, 0));
}

void Enemy::Draw()
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

	//DrawCapsule3D(p1, p2, rad, 40, 8, GetColor(255, 255, 0), FALSE);
}

/// <summary>
/// �������Ă����true
/// </summary>
bool Enemy::HitCollision(VECTOR _cpos, float _crad, bool _downAttack)
{
	float len;//�ŒZ����
	float arad = _crad + rad;//���a�i���́j+ ���a�i�J�v�Z���j
	Player *pl = GetScene()->FindGameObject<Player>();
	VECTOR PlayerPosition = VNorm(VSub(pl->GetPosition(), Position));
	//�J�v�Z����
	//��
	if (_cpos.y >= p2.y)
	{
		len = VSize(VSub(p2, _cpos));
		//���������a�i���́j+ ���a�i�J�v�Z���j�ȉ� 
		if (len <= arad)
		{
			if (_downAttack == true)
			{
				Rotation.y = atan2(PlayerPosition.x* -1.0f, PlayerPosition.z* -1.0f);
				animation->Start(ANIM_ID::DOWN, false);
				status = STATE_DOWN;
				return true;
			}
			animation->Start(ANIM_ID::KNOCKBACK, false);
			animation->ResetTime();
			status = STATE_WASATTACK;
			return true;
		}
		return false;
	}
	//��
	if (_cpos.y <= p1.y)
	{
		len = VSize(VSub(p1, _cpos));
		//���������a�i���́j+ ���a�i�J�v�Z���j�ȉ� 
		if (len <= arad)
		{
			if (_downAttack == true)
			{
				Rotation.y = atan2(PlayerPosition.x* -1.0f, PlayerPosition.z* -1.0f);
				animation->Start(ANIM_ID::DOWN, false);
				status = STATE_DOWN;
				return true;
			}
			animation->Start(ANIM_ID::KNOCKBACK, false);
			animation->ResetTime();
			status = STATE_WASATTACK;
			return true;
		}
		return false;
	}

	//�����𑪂�
	len = VSize(VSub(VGet(p1.x, 0.0f, p1.z), VGet(_cpos.x, 0.0f, _cpos.z)));

	//������蔼�a�i���́j+ ���a�i�J�v�Z���j�̕����傫����� 
	if (arad > len)
	{
		if (_downAttack == true)
		{
			Rotation.y = atan2(PlayerPosition.x* -1.0f, PlayerPosition.z* -1.0f);
			animation->Start(ANIM_ID::DOWN, false);
			status = STATE_DOWN;
			return true;
		}
		animation->Start(ANIM_ID::KNOCKBACK, false);
		animation->ResetTime();
		status = STATE_WASATTACK;
		return true;
	}
	return false;
}

VECTOR Enemy::CollisionPlayer(VECTOR center, float r, VECTOR vel)
{
	VECTOR v2 = VSub(Position, center); // �v���C���[����G�ւ̃x�N�g��
	v2.y = 0.0f; // Y�̍���0�ɂ��Ă���(�~���œ��Ă�)

	if (VSize(v2) > r + rad) { // �������ĂȂ�
		return center;
	}
	v2 = VNorm(v2); // �������P�ɂ���
	float push = VDot(vel, v2); // ��������
	if (push <= 0.0f) // ���Ό����Ȃ̂ŏ������Ȃ�
		return center;
	VECTOR enePush = VScale(v2, push); // �G��������
	VECTOR playerPush = VSub(vel, enePush);
	Position = VAdd(Position, enePush);
	center = VSub(center, vel); // �ړ��O�̈ʒu�ɖ߂�
	return VAdd(center, playerPush); // �ړ���̈ʒu���Čv�Z
}

void Enemy::Move()
{
	// �ҋ@�A�j���[�V�������Đ�
	animation->Start(ANIM_ID::NEUTRAL, true);
}

void Enemy::Walk()
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

	MATRIX rotY = MGetRotY(Rotation.y); //��]�s������


	// �ړ��̏���
	VECTOR vel = VTransform(VGet(0, 0, -1.0f), rotY);	//��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	//���W�Ɉړ��ʂ𑫂�

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
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = 1;
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

void Enemy::Run()
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
	VECTOR vel = VTransform(VGet(0, 0, -5.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
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
		animation->Start(ANIM_ID::TACKLE, false);
		beforestatus = STATE_DASHATTACK;
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

void Enemy::Jump()
{
	// �n�ʂɂ����珈������߂�
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = beforestatus;
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
		VECTOR vel = VTransform(VGet(0, 0, -1.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
		Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
	}
	if (beforestatus == STATE_RUN)
	{
		VECTOR vel = VTransform(VGet(0, 0, -5.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
		Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
	}

	//�@�W�����v�A���U�������s
	// �A���U�������s
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = 1;
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

void Enemy::ComboAttack()
{
	// �A������ڂ��ōU�����ς��
	switch (ComboNum)
	{
	case 1:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 15.0f)
			{
				ComboNum = 2;
				animation->Start(ANIM_ID::ATTACK_2, false);
			}
		}
		if (animation->IsFinish() == true)
		{
			status = STATE_NEUTRAL;
		}
		break;
	case 2:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 5.0f)
			{
				ComboNum = 3;
				animation->Start(ANIM_ID::ATTACK_3, false);
			}
		}
		if (animation->IsFinish() == true)
		{
			status = STATE_NEUTRAL;
		}
		break;
	case 3:
		if (animation->IsFinish() == true)
		{
			status = STATE_NEUTRAL;
		}
		break;
	}
}

void Enemy::DashAttack()
{
	if (animation->IsFinish() == true)
	{
		status = STATE_NEUTRAL;
	}
	//�ړ��ʂ̕������Ă�������ɐi�߂�

	if (animation->GetFrameTime() < 5.0f || animation->GetFrameTime() > 20.0f)
	{
		return;
	}

	MATRIX rotY = MGetRotY(Rotation.y);

	VECTOR vel = VTransform(VGet(0, 0, -3.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
	Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�

}

void Enemy::JumpComboAttack()
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
	case 1:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 15.0f)
			{
				ComboNum = 2;
				animation->Start(ANIM_ID::ATTACK_2, false);
			}
		}
		if (animation->IsFinish() == true)
		{
			status = STATE_JUMP;
		}
		break;
	case 2:
		if (CheckHitKey(KEY_INPUT_RETURN) != 0)
		{
			if (animation->GetFrameTime() > 5.0f)
			{
				ComboNum = 3;
				animation->Start(ANIM_ID::ATTACK_3, false);
			}
		}
		if (animation->IsFinish() == true)
		{
			status = STATE_JUMP;
		}
		break;
	case 3:
		if (animation->IsFinish() == true)
		{
			status = STATE_JUMP;
		}
		break;
	}
}

void Enemy::Dodge()
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

void Enemy::Guard()
{
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		return;
	}
	status = STATE_NEUTRAL;
}

void Enemy::JumpGuard()
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

void Enemy::WasAttack()
{
	if (animation->IsFinish() == true)
	{
		animation->Start(ANIM_ID::NEUTRAL, true);
		status = STATE_NEUTRAL;
	}
}

void Enemy::Down()
{

	MATRIX rotY = MGetRotY(Rotation.y);
	if (animation->GetFrameTime() < 35.0f)
	{
		VECTOR vel = VTransform(VGet(0, 0, 1.0f), rotY);	// ��]�s����g�����x�N�g���̕ϊ�
		Position = VAdd(Position, vel);	// ���W�Ɉړ��ʂ𑫂�
	}
	
	
	if (animation->IsFinish() == true)
	{
		animation->Start(ANIM_ID::STANDUP, false);
		status = STATE_STANDUP;
	}
}

void Enemy::DownLoop()
{
}

void Enemy::StandUp()
{
	if (animation->IsFinish() == true)
	{
		animation->Start(ANIM_ID::NEUTRAL, false);
		status = STATE_NEUTRAL;
	}
}

void Enemy::HitField()
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

void Enemy::SetCamera()
{
}

void Enemy::SetCharacterRotationY()
{
}
