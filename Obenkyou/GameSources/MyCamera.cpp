#include "stdafx.h"
#include "Project.h"

namespace basecross {

	//Impl
	struct MyCamera::Impl {
		weak_ptr<GameObject> m_TargetObject;	//�ڕW�ƂȂ�I�u�W�F�N�g
		float m_ToTargetLerp;	//�ڕW��ǂ�������ۂ̕�Ԓl


		float m_RadY;
		float m_RadXZ;
		//�J�����̏㉺�X�s�[�h
		float m_CameraUpDownSpeed;
		//�J�����������鉺���p�x
		float m_CameraUnderRot;
		float	m_Arm;
		//�r�̒����̐ݒ�
		float m_MaxArm;
		float m_MinArm;
		//��]�X�s�[�h
		float m_RotSpeed;
		//�Y�[���X�s�[�h
		float m_ZoomSpeed;

		Impl() :
			m_ToTargetLerp(1.0f),
			m_RadY(0.0f),
			m_RadXZ(0),
			m_CameraUpDownSpeed(0.0f),
			m_CameraUnderRot(0.0f),
			m_Arm(5.0f),
			m_MaxArm(20.0f),
			m_MinArm(2.0f),
			m_RotSpeed(1.0f),
			m_ZoomSpeed(0.1f)
		{}
		~Impl() {}
	};


	//GameStage�p�̃J����
	MyCamera::MyCamera() :
		Camera(),
		pImpl(new Impl())
	{
	}
	MyCamera::~MyCamera() {}
	//�A�N�Z�T
	shared_ptr<GameObject> MyCamera::GetTargetObject() const {
		if (!pImpl->m_TargetObject.expired()) {
			return pImpl->m_TargetObject.lock();
		}
		return nullptr;
	}

	void MyCamera::SetTargetObject(const shared_ptr<GameObject>& Obj) {
		pImpl->m_TargetObject = Obj;
	}

	float MyCamera::GetToTargetLerp() const {
		return pImpl->m_ToTargetLerp;
	}
	void MyCamera::SetToTargetLerp(float f) {
		pImpl->m_ToTargetLerp = f;
	}

	void MyCamera::OnUpdate() {
		//Scene�̎擾
		auto ScenePtr = App::GetApp()->GetScene<Scene>();

		auto CntlVec = App::GetApp()->GetInputDevice().GetControlerVec();
		//�O��̃^�[������̎���
		float ElapsedTime = App::GetApp()->GetElapsedTime();
		Vector3 NewEye = GetEye();
		Vector3 NewAt = GetAt();
		//�v�Z�Ɏg�����߂̘r�p�x�i�x�N�g���j
		Vector3 ArmVec = NewEye - NewAt;
		//���K�����Ă���
		ArmVec.Normalize();
		//if (CntlVec[0].bConnected) {

		//�N�I�[�^�j�I����Y��]�i�܂�XZ�x�N�g���̒l�j���v�Z
		Quaternion QtXZ;
		QtXZ.RotationAxis(Vector3(0, 0.0f, 0), pImpl->m_RadXZ);
		QtXZ.Normalize();
		//�ړ���s�̍s��v�Z���邱�ƂŁAXZ�̒l���Z�o
		Matrix4X4 Mat;
		Mat.STRTransformation(
			Vector3(1.0f, 1.0f, 1.0f),
			Vector3(0.0f, 0.0f, -1.0f),
			QtXZ
		);

		Vector3 PosXZ = Mat.PosInMatrixSt();
		//XZ�̒l���킩�����̂Řr�p�x�ɑ��
		ArmVec.x = PosXZ.x;
		ArmVec.z = PosXZ.z;
		//�r�p�x�𐳋K��
		ArmVec.Normalize();

		auto TargetPtr = GetTargetObject();
		if (TargetPtr) {
			//�ڎw�������ꏊ
			Matrix4X4 ToAtMat = TargetPtr->GetComponent<Transform>()->GetWorldMatrix();
			Vector3 ToAt = ToAtMat.PosInMatrixSt();
			NewAt = Lerp::CalculateLerp(GetAt(), ToAt, 0, 1.0f, 2.0f, Lerp::Linear);
		}

		////�ڎw�������ꏊ�ɃA�[���̒l�Ƙr�x�N�g����Eye�𒲐�
		/*NewEye = NewAt + ArmVec * pImpl->m_Arm;

		SetEye(NewEye.x + 4.0f, NewEye.y + 2.0f, NewEye.z - 5.0f);
		SetAt(NewAt.x + 4.0f, NewAt.y + 2.0f, NewAt.z);
		Camera::OnUpdate();*/

	}
}