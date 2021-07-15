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
	static const std::string MODEL_CHARACTER = "data\\model\\Player\\うさいだーまっ_.pmx";
	static const std::string FRAMENAME_MOVE = "全ての親";
	static const VECTOR POSITION = VGet(-50, 0, 0);	//初期座標
	static const VECTOR ROTATION = VGet(0.0f, 270.0f * DX_PI_F / 180.0f, 0.0f);	//初期回転値
	static const float ANGLE_YAW = 180.0f * DX_PI_F / 180.0f; //カメラの初期回転値（横）
	static const float ANGLE_PITCH = 0.0f;					  //カメラの初期回転値（縦）
	static const float CAMERA_RADIUS = 80.0f;	// キャラクターからカメラまでの距離（半径）
	static const float RADIUS = 5.0f;			// 敵を動かす当たり判定の半径
	static const float SPEED_WALK = 1.0f;		// 歩きの移動量
	static const float SPEED_RUN = 5.0f;		// 走りの移動量
	static const float JUMP_POWER = 3.0f;		// ジャンプ力
	static const float JUMP_DOWNPOWER = 0.1f;	// ジャンプ力を下げる力
	static const float SPEED_CAMERA = 5.0f * DX_PI_F / 180.0f;		// カメラの回転スピード
	// 攻撃の当たり判定のアタッチするフレームの名前
	static const std::string FRAMENAME_ATTACK[Player::ATTACK::MAX] =
	{
		"左つま先",
		"左手首",
		"右手首",
		"左肩"
	};
	// 攻撃の当たり判定の半径
	static const float RADIUS_ATTACK[Player::ATTACK::MAX] =
	{
		2.0f,
		2.0f,
		2.0f,
		3.0f,
	};
	// その攻撃はダウンするかどうか(trueでダウンする)
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
	// モデルの読み込み
	hModel = MV1LoadModel(MODEL_CHARACTER.c_str());
	assert(hModel > 0);

	// アニメーションの生成
	animation = new Animation(hModel);
	animation->Start(ANIM_ID::NEUTRAL, true);

	// アニメーションで移動をしているフレームの番号を検索する
	MoveAnimFrameIndex = MV1SearchFrame(hModel, FRAMENAME_MOVE.c_str());

	// アニメーションで移動をしているフレームを無効にする
	MV1SetFrameUserLocalMatrix(hModel, MoveAnimFrameIndex, MGetIdent());

	// ３Ｄモデルの座標を初期化
	Position = POSITION;
	Rotation = ROTATION;

	//カメラの回転値
	YawAngle = ANGLE_YAW;
	PitchAngle = ANGLE_PITCH;

	jumpPower = JUMP_POWER;
	isJump = false;

	// 攻撃の当たり判定のアタッチするフレームの番号を検索
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
	// モデルの削除
	if (hModel > 0)
		MV1DeleteModel(hModel);
}

void Player::Start()
{
	// オブジェクトの読み込み
	enemy = GetScene()->FindGameObject<Enemy>();
	assert(enemy != nullptr);

	Cm = GetScene()->FindGameObject<Camera>();
	assert(Cm != nullptr);

	effectManager = GetScene()->FindGameObject<EffectManager>();
	assert(effectManager != nullptr);
}

void Player::Update()
{
	// 移動量を計算するため更新する前の座標を格納
	TempPosition = Position;
	// 地面との当たり判定
	HitField();

	if (CheckHitKey(KEY_INPUT_SPACE))
	{
		
	}

	// ステータスの更新
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

	// 当たり判定の処理
	for (int i = 0; i < ATTACK::MAX; i++)
	{
		// その当たり判定が有効かどうか
		if (Collision[i].isEnable == false)
		{
			continue;
		}
		//当たり判定の座標を更新
		Collision[i].Position = MV1GetFramePosition(hModel, Collision[i].FrameIndex);
		// 当たったらtrue
		if (enemy->HitCollision(Collision[i].Position, Collision[i].Radius, Collision[i].DownAttack))
		{
			//当たり判定を無効に
			Collision[i].isEnable = false;
		}
	}

	// 移動量を計算
	Vel = VSub(Position, TempPosition);

	// 敵をずらす
	enemy->CollisionPlayer(Position, RADIUS, Vel);

	// カメラの更新
	SetCamera();

	// アニメーションのアップデート
	animation->Update();
}

void Player::Draw()
{
	MATRIX m;

	//// 移動の行列を作る
	MATRIX mTrans = MGetTranslate(Position);

	//// Y軸回転の行列を作る
	MATRIX mRotY = MGetRotY(Rotation.y);

	//// 回転行列と平行移動行列
	m = MMult(mRotY, mTrans);

	//// キャラクターモデルの行列の設定
	MV1SetMatrix(hModel, m);

	// キャラクターモデル描画
	MV1DrawModel(hModel);

	//当たり判定の描画
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
	// 歩行を実行
	if (CheckHitKey(KEY_INPUT_W) ||
		CheckHitKey(KEY_INPUT_A) ||
		CheckHitKey(KEY_INPUT_S) ||
		CheckHitKey(KEY_INPUT_D))
	{
		beforestatus = STATE_MOVE;
		status = STATE_WALK;
		return;
	}

	// ジャンプを実行
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		// ジャンプアニメーションを再生
		animation->Start(ANIM_ID::JUMP, true);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_MOVE;
		status = STATE_JUMP;
		return;
	}

	// 連続攻撃を実行
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z);//敵の方を向く
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		Collision[ATTACK::NORMAL_1].isEnable = true;
		status = STATE_COMBOATTACK;
		return;
	}
	// 回避を実行
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}

	// ガードを実行
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		animation->Start(ANIM_ID::GUARD, true);
		status = STATE_GUARD;
		return;
	}

	// 待機アニメーションを再生
	animation->Start(ANIM_ID::NEUTRAL, true);
}

void Player::Walk()
{
	// 何も操作していなければ待機状態に戻す
	if (CheckHitKey(KEY_INPUT_W) == 0 &&
		CheckHitKey(KEY_INPUT_A) == 0 &&
		CheckHitKey(KEY_INPUT_S) == 0 &&
		CheckHitKey(KEY_INPUT_D) == 0)
	{
		beforestatus = STATE_WALK;
		status = STATE_NEUTRAL;
		return;
	}

	// キーを押すことで走りの処理を実行
	if (CheckHitKey(KEY_INPUT_LSHIFT) != 0)
	{
		beforestatus = STATE_WALK;
		status = STATE_RUN;
		return;
	}

	// 歩きアニメーションを再生
	animation->Start(ANIM_ID::WALK, true);

	// カメラの向きに合わせてキャラクターの向きを変える
	SetCharacterRotationY();

	MATRIX rotY = MGetRotY(Rotation.y); // Y軸の回転行列を作る

	// 移動の処理
	VECTOR vel = VTransform(VGet(0, 0, -SPEED_WALK), rotY);	// 回転行列を使ったベクトルの変換
	Position = VAdd(Position, vel);	// 座標に移動量を足す

	// ジャンプを実行
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		// ジャンプアニメーションを再生
		animation->Start(ANIM_ID::JUMP, false);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_WALK;
		status = STATE_JUMP;
		return;
	}

	// 連続攻撃を実行
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z);
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		Collision[ATTACK::NORMAL_1].isEnable = true;
		status = STATE_COMBOATTACK;
		return;
	}

	//回避を実行
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}


	// ガードを実行
	if (CheckHitKey(KEY_INPUT_G) != 0)
	{
		animation->Start(ANIM_ID::GUARD, false);
		status = STATE_GUARD;
		return;
	}
}

void Player::Run()
{
	// キーを押していなければ歩きの処理を実行
	if (CheckHitKey(KEY_INPUT_LSHIFT) == 0)
	{
		beforestatus = STATE_RUN;
		status = STATE_WALK;
	}

	// 走りアニメーションを再生
	animation->Start(ANIM_ID::RUN, true);

	// カメラの向きに合わせてキャラクターの向きを変える
	SetCharacterRotationY();

	MATRIX rotY = MGetRotY(Rotation.y); //回転行列を作る

	// 移動の処理
	VECTOR vel = VTransform(VGet(0, 0, -SPEED_RUN), rotY);	// 回転行列を使ったベクトルの変換
	Position = VAdd(Position, vel);	// 座標に移動量を足す

	// ジャンプを実行
	if (CheckHitKey(KEY_INPUT_SPACE) != 0)
	{
		//ジャンプアニメーションを再生
		animation->Start(ANIM_ID::JUMP, false);
		jumpPower = JUMP_POWER;
		Position.y += jumpPower;
		isJump = true;
		beforestatus = STATE_RUN;
		status = STATE_JUMP;
	}

	// ダッシュ攻撃を実行
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		Rotation.y = atan2(Position.x - enemy->GetPosition().x, Position.z - enemy->GetPosition().z); //敵の方を向く
		animation->Start(ANIM_ID::TACKLE, false);
		beforestatus = STATE_DASHATTACK;
		Collision[ATTACK::TACKLE_1].isEnable = true;
		status = STATE_DASHATTACK;
		return;
	}

	//回避を実行
	if (CheckHitKey(KEY_INPUT_C) != 0)
	{
		animation->Start(ANIM_ID::DODGE, false);
		status = STATE_DODGE;
		return;
	}
}

void Player::Jump()
{
	// 地面についたら処理をやめる
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// 処理
	// ジャンプ力を減らす
	jumpPower -= JUMP_DOWNPOWER;
	//座標にジャンプ力を与える
	Position.y += jumpPower;

	MATRIX rotY = MGetRotY(Rotation.y);

	// 座標を移動
	if (beforestatus == STATE_WALK)
	{
		VECTOR vel = VTransform(VGet(0, 0, -SPEED_WALK), rotY);	// 回転行列を使ったベクトルの変換
		Position = VAdd(Position, vel);	// 座標に移動量を足す
	}
	if (beforestatus == STATE_RUN)
	{
		VECTOR vel = VTransform(VGet(0, 0, -SPEED_RUN), rotY);	// 回転行列を使ったベクトルの変換
		Position = VAdd(Position, vel);	// 座標に移動量を足す
	}

	//　ジャンプ連続攻撃を実行
	// 連続攻撃を実行
	if (CheckHitKey(KEY_INPUT_RETURN) != 0)
	{
		animation->Start(ANIM_ID::ATTACK_1, false);
		ComboNum = ATTACK::NORMAL_1;
		status = STATE_JUMPCOMBOATTACK;
		return;
	}

	// ガードを実行
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
	VECTOR vel = VTransform(VGet(0, 0, -1.0f), rotY);	//回転行列を使ったベクトルの変換

	// 連続何回目かで攻撃が変わる
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

	//移動量の分向いている方向に進める	
	MATRIX rotY = MGetRotY(Rotation.y);
	VECTOR vel = VTransform(VGet(0, 0, -3.0f), rotY);	// 回転行列を使ったベクトルの変換
	Position = VAdd(Position, vel);	// 座標に移動量を足す
}

void Player::JumpComboAttack()
{
	// 地面についたら処理をやめる
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// 処理
	// ジャンプ力を減らす
	jumpPower -= JUMP_DOWNPOWER;
	Position.y += jumpPower;

	// 連続何回目かで攻撃が変わる
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
	VECTOR vel = VTransform(VGet(0, 0, -10.0f), rotY);	// 回転行列を使ったベクトルの変換
	Position = VAdd(Position, vel);	// 座標に移動量を足す
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
	// 地面についたら処理をやめる
	if (Position.y <= hit.y)
	{
		isJump = false;
		status = STATE_NEUTRAL;
	}

	// 処理
	// ジャンプ力を減らす
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
	// 地面との当たり判定の為のRayを作成
	VECTOR upper = VAdd(Position, VGet(0, 1000, 0));
	VECTOR lower = VAdd(Position, VGet(0, -1000, 0));

	// ステージを読み込み
	Field* field = GetScene()->FindGameObject<Field>();

	//　地面との当たり判定
	if (field->CollisionLine(&hit, upper, lower))
	{
		//　ジャンプしていなければ座標を地面と座標を合わせる
		if (isJump == false)
		{
			Position = hit;
		}
	}
	else
	{
		// 足元に地面が無ければ降下
		jumpPower -= JUMP_DOWNPOWER;
		Position.y += jumpPower;
	}
}

void Player::SetCamera()
{
	// カメラ操作
	float radius = CAMERA_RADIUS; //半径
	// 横
	if (CheckHitKey(KEY_INPUT_RIGHT))YawAngle -= SPEED_CAMERA;
	if (CheckHitKey(KEY_INPUT_LEFT))YawAngle += SPEED_CAMERA;
	// 縦
	if (CheckHitKey(KEY_INPUT_UP))PitchAngle += SPEED_CAMERA;
	if (CheckHitKey(KEY_INPUT_DOWN))PitchAngle -= SPEED_CAMERA;

	CPos.y = sinf(PitchAngle) * radius;	// y軸の座標を更新
	float radius2 = cosf(PitchAngle) * radius;			// xz軸で使う半径を作成
	CPos.x = cos(YawAngle) * radius2;		// x軸の座標を更新
	CPos.z = sinf(YawAngle) * radius2;	// z軸の座標を更新
	Cm->SetTarget(VSub(VGet(Position.x, Position.y + 30.0f, Position.z), CPos));		// 向く位置を設定
	Cm->SetPosition(VAdd(VGet(Position.x, Position.y + 30.0f, Position.z), CPos));			// カメラの位置を設定
}

void Player::SetCharacterRotationY()
{
	// カメラの向きに合わせてキャラクターの向きを変える
	if (CheckHitKey(KEY_INPUT_W) != 0)
	{
		// 前
		Rotation.y = atan2(CPos.x, CPos.z);
		if (CheckHitKey(KEY_INPUT_A) != 0)
		{
			// 左前
			Rotation.y = atan2(CPos.x, CPos.z) + 315.0f * DX_PI_F / 180.0f;
		}
		if (CheckHitKey(KEY_INPUT_D) != 0)
		{
			// 右前
			Rotation.y = atan2(CPos.x, CPos.z) + 45.0f * DX_PI_F / 180.0f;
		}
	}
	else if (CheckHitKey(KEY_INPUT_S) != 0)
	{
		// 後
		Rotation.y = atan2(CPos.x, CPos.z) + 180.0f * DX_PI_F / 180.0f;
		if (CheckHitKey(KEY_INPUT_A) != 0)
		{
			// 左後
			Rotation.y = atan2(CPos.x, CPos.z) + 225.0f * DX_PI_F / 180.0f;
		}
		if (CheckHitKey(KEY_INPUT_D) != 0)
		{
			// 右後
			Rotation.y = atan2(CPos.x, CPos.z) + 135.0f * DX_PI_F / 180.0f;
		}
	}
	else if (CheckHitKey(KEY_INPUT_A) != 0)
	{
		// 左
		Rotation.y = atan2(CPos.x, CPos.z) + 270.0f * DX_PI_F / 180.0f;
	}
	else if (CheckHitKey(KEY_INPUT_D) != 0)
	{
		// 右
		Rotation.y = atan2(CPos.x, CPos.z) + 90.0f * DX_PI_F / 180.0f;
	}
}
