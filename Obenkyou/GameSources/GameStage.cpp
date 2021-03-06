/*!
@file GameStage.cpp
@brief ゲームステージ実体
*/

#include "stdafx.h"
#include "Project.h"

namespace basecross {

	//--------------------------------------------------------------------------------------
	//	ゲームステージクラス実体
	//--------------------------------------------------------------------------------------

	//ビューとライトの作成
	void GameStage::CreateViewLight() {
		auto PtrView = CreateView<SingleView>();
		//ビューのカメラの設定
		auto PtrLookAtCamera = ObjectFactory::Create<MyCamera>();
		PtrView->SetCamera(PtrLookAtCamera);
		PtrLookAtCamera->SetEye(Vector3(0.0f, 13.0f, -5.0f));
		PtrLookAtCamera->SetAt(Vector3(0.0f, -10.0f, 0.0f));
		//マルチライトの作成
		auto PtrMultiLight = CreateLight<MultiLight>();
		//デフォルトのライティングを指定
		PtrMultiLight->SetDefaultLighting();
	}


	//プレートの作成
	void GameStage::CreatePlate() {
		//ステージへのゲームオブジェクトの追加
		auto Ptr = AddGameObject<GameObject>();
		auto PtrTrans = Ptr->GetComponent<Transform>();
		Quaternion Qt(Vector3(1.0f, 0, 0), XM_PIDIV2);
		Qt.RotationRollPitchYawFromVector(Vector3(XM_PIDIV2, 0, 0));
		PtrTrans->SetScale(50.0f, 50.0f, 1.0f);
		PtrTrans->SetQuaternion(Qt);
		PtrTrans->SetPosition(0.0f, 0.0f, 0.0f);

		auto ColPtr = Ptr->AddComponent<CollisionRect>();
		//描画コンポーネントの追加
		auto DrawComp = Ptr->AddComponent<BcPNTStaticDraw>();
		//描画コンポーネントに形状（メッシュ）を設定
		DrawComp->SetMeshResource(L"DEFAULT_SQUARE");
		DrawComp->SetFogEnabled(true);
		//自分に影が映りこむようにする
		DrawComp->SetOwnShadowActive(true);

		//描画コンポーネントテクスチャの設定
		DrawComp->SetTextureResource(L"SKY_TX");
	}
	
	//プレイヤーの作成
	void GameStage::CreatePlayer() {
		//プレーヤーの作成
		auto PlayerPtr = AddGameObject<Player>();
		//シェア配列にプレイヤーを追加
		SetSharedGameObject(L"Player", PlayerPtr);
	}
	//Boxの作成

	void GameStage::CreateSpark() {
		auto MultiSparkPtr = AddGameObject<MultiSpark>();

		SetSharedGameObject(L"MultiSpark", MultiSparkPtr);
	
	
	}




	void GameStage::CreateBox() {
		
		
		vector< vector<Vector3> > Vec = {
			{
				Vector3(1.0f, 1.0f, 1.0f),	//スケール
				Vector3(0.0f, 0.0f, 0.0f),	//ろーてーしょん
				Vector3(5.0f, 0.5f, 0.0f)	//ポジション
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//スケール
				Vector3(0.0f, 0.0f, 0.0f),	//ろーてーしょん
				Vector3(6.0f, 0.5f, 1.0f)	//ポジション
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//スケール
				Vector3(0.0f, 0.0f, 0.0f),	//ろーてーしょん
				Vector3(4.0f, 0.5f, -1.0f)	//ポジション
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//スケール
				Vector3(0.0f, 0.0f, 0.0f),	//ろーてーしょん
				Vector3(4.0f, 0.5f, 1.0f)	//ポジション
			},
			{
				Vector3(1.0f, 1.0f, 1.0f),	//スケール
				Vector3(0.0f, 0.0f, 0.0f),	//ろーてーしょん
				Vector3(6.0f, 0.5f, -1.0f)	//ポジション
			},
		};
		//オブジェクトの作成
		for (auto v : Vec) {
			AddGameObject<FixedBox>(v[0], v[1], v[2]);
		}
	}
	
	void GameStage::CreateSprite() {
		AddGameObject<WallSprite>(
			L"WALL_TX",
			true,
			Vector2(200.0f, 200.0f),
			Vector2(440.0f, -200.0f));
	}


	void GameStage::OnCreate() {
		try {
			//ビューとライトの作成
			CreateViewLight();
			//プレートの作成
			CreatePlate();
			//プレーヤーの作成
			CreatePlayer();
			//Boxの作成
			//CreateBox();
			//Spriteの作成
			CreateSprite();
			CreateSpark();
		}
		catch (...) {
			throw;
		}
		
	}
	void GameStage::OnUpdate() {
		auto Mouse = App::GetApp()->GetInputDevice().GetKeyState();
		auto ScenePtr = App::GetApp()->GetScene<Scene>();
		if (Mouse.m_bPressedKeyTbl[VK_RBUTTON]) {
			PostEvent(0.0f, GetThis<GameStage>(), ScenePtr, L"Tostage_title");
		}
	}
}
//end basecross
