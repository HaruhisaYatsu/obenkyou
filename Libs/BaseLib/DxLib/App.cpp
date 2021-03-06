/*!
@file App.cpp
@brief アプリケーションクラス。入力機器等実体
@copyright Copyright (c) 2017 WiZ Tamura Hiroki,Yamanoi Yasushi.
*/
#include "stdafx.h"


namespace basecross {

	//--------------------------------------------------------------------------------------
	//	struct AudioManager::Impl;
	//	用途: Implクラス
	//--------------------------------------------------------------------------------------
	struct AudioManager::Impl {
		HWND m_hWnd;
		bool m_audioAvailable;
		ComPtr<IXAudio2>    m_musicEngine;
		ComPtr<IXAudio2>    m_soundEffectEngine;
		IXAudio2MasteringVoice* m_musicMasteringVoice;
		IXAudio2MasteringVoice* m_soundEffectMasteringVoice;
		Impl(HWND hWnd) :
			m_hWnd(hWnd),
			m_audioAvailable{ false },
			m_musicMasteringVoice(nullptr),
			m_soundEffectMasteringVoice(nullptr)
		{}
		~Impl() {}
	};



	//--------------------------------------------------------------------------------------
	//	class AudioManager;
	//--------------------------------------------------------------------------------------
	AudioManager::AudioManager(HWND hWnd) :
		pImpl(new Impl(hWnd))
	{
	}
	AudioManager::~AudioManager() {
		if (pImpl->m_soundEffectEngine) {
			pImpl->m_soundEffectEngine->StopEngine();
			pImpl->m_soundEffectEngine = nullptr;
		}
		if (pImpl->m_musicEngine) {
			pImpl->m_musicEngine->StopEngine();
			pImpl->m_musicEngine = nullptr;
		}
	}

	void AudioManager::AudioUnAvailableMassage() {
		MessageBox(pImpl->m_hWnd, L"オーディオが取得できなかったので、音声なしで実行します", L"警告", MB_OK);
	}


	void AudioManager::CreateDeviceIndependentResources()
	{
		UINT32 flags = 0;
		HRESULT hr = XAudio2Create(&pImpl->m_musicEngine, flags);
		if (FAILED(hr)) {
			pImpl->m_audioAvailable = false;
			AudioUnAvailableMassage();
			return;
		}

#if defined(_DEBUG)
		XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = { 0 };
		debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
		debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS;
		pImpl->m_musicEngine->SetDebugConfiguration(&debugConfiguration);
#endif
		hr = pImpl->m_musicEngine->CreateMasteringVoice(&pImpl->m_musicMasteringVoice);
		if (FAILED(hr))
		{
			pImpl->m_audioAvailable = false;
			AudioUnAvailableMassage();
			return;
		}

		hr = XAudio2Create(&pImpl->m_soundEffectEngine, flags);
		if (FAILED(hr)) {
			pImpl->m_audioAvailable = false;
			AudioUnAvailableMassage();
			return;
		}
#if defined(_DEBUG)
		pImpl->m_soundEffectEngine->SetDebugConfiguration(&debugConfiguration);
#endif

		hr = pImpl->m_soundEffectEngine->CreateMasteringVoice(&pImpl->m_soundEffectMasteringVoice);
		if (FAILED(hr)) {
			pImpl->m_audioAvailable = false;
			AudioUnAvailableMassage();
			return;
		}
		pImpl->m_audioAvailable = true;
	}

	IXAudio2* AudioManager::GetMusicEngine()const
	{
		return pImpl->m_musicEngine.Get();
	}

	IXAudio2* AudioManager::GetSoundEffectEngine()const
	{
		return pImpl->m_soundEffectEngine.Get();
	}

	void AudioManager::SuspendAudio()
	{
		if (pImpl->m_audioAvailable)
		{
			pImpl->m_musicEngine->StopEngine();
			pImpl->m_soundEffectEngine->StopEngine();
		}
	}

	void AudioManager::ResumeAudio()
	{
		if (pImpl->m_audioAvailable)
		{
			ThrowIfFailed(
				pImpl->m_musicEngine->StartEngine(),
				L"音楽用エンジンのスタートに失敗しました",
				L"m_musicEngine->StartEngine()",
				L"AudioManager::CreateDeviceIndependentResources()"
			);
			ThrowIfFailed(
				pImpl->m_soundEffectEngine->StartEngine(),
				L"サウンド用エンジンのスタートに失敗しました",
				L"m_soundEffectEngine->StartEngine()",
				L"AudioManager::CreateDeviceIndependentResources()"
			);
		}
	}

	bool AudioManager::IsAudioAvailable()const {
		return pImpl->m_audioAvailable;
	}



	//--------------------------------------------------------------------------------------
	//	struct AudioResource::Impl;
	//	用途: Implイディオム
	//--------------------------------------------------------------------------------------
	struct AudioResource::Impl {
		WAVEFORMATEX m_WaveFormat;	//ウェブフォーマット
		vector<byte> m_SoundData;	//データ
		wstring m_FileName;		//ファイルへのパス
		Impl(const wstring& FileName) :
			m_FileName(FileName) {}
		~Impl() {}
		//ミューテックス
		std::mutex Mutex;

	};



	//--------------------------------------------------------------------------------------
	//	class AudioResource : public BaseResource;
	//	用途: オーディオリソース（wavなど）のラッピングクラス
	//--------------------------------------------------------------------------------------
	AudioResource::AudioResource(const wstring& FileName) :
		BaseResource(),
		pImpl(new Impl(FileName))
	{
		try {
			if (!App::GetApp()->GetAudioManager()->IsAudioAvailable()) {
				//マネージャが無効ならリターン
				return;
			}

			ThrowIfFailed(
				MFStartup(MF_VERSION),
				L"MediaFoundationの初期化に失敗しました",
				L"MFStartup(MF_VERSION)",
				L"AudioResource::AudioResource()"
			);

			ComPtr<IMFSourceReader> reader;

			ThrowIfFailed(
				MFCreateSourceReaderFromURL(FileName.c_str(), nullptr, &reader),
				L"サウンドリーダーの作成に失敗しました",
				L"MFCreateSourceReaderFromURL(FileName.c_str(),nullptr,&reader)",
				L"AudioResource::AudioResource()"
			);

			// Set the decoded output format as PCM.
			// XAudio2 on Windows can process PCM and ADPCM-encoded buffers.
			// When using MediaFoundation, this sample always decodes into PCM.
			Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

			ThrowIfFailed(
				MFCreateMediaType(&mediaType),
				L"メディアタイプの作成に失敗しました",
				L"MFCreateMediaType(&mediaType)",
				L"AudioResource::AudioResource()"
			);

			ThrowIfFailed(
				mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio),
				L"メディアGUIDの設定に失敗しました",
				L"mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)",
				L"AudioResource::AudioResource()"
			);

			ThrowIfFailed(
				mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM),
				L"メディアサブGUIDの設定に失敗しました",
				L"mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)",
				L"AudioResource::AudioResource()"
			);

			ThrowIfFailed(
				reader->SetCurrentMediaType(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, mediaType.Get()),
				L"リーダーへのメディアタイプの設定に失敗しました",
				L"reader->SetCurrentMediaType(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, mediaType.Get())",
				L"AudioResource::AudioResource()"
			);


			// Get the complete WAVEFORMAT from the Media Type.
			Microsoft::WRL::ComPtr<IMFMediaType> outputMediaType;

			ThrowIfFailed(
				reader->GetCurrentMediaType(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &outputMediaType),
				L"出力用のメディアタイプの設定に失敗しました",
				L"reader->GetCurrentMediaType(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &outputMediaType)",
				L"AudioResource::AudioResource()"
			);

			UINT32 size = 0;
			WAVEFORMATEX* waveFormat;

			ThrowIfFailed(
				MFCreateWaveFormatExFromMFMediaType(outputMediaType.Get(), &waveFormat, &size),
				L"ウェブフォーマットの設定に失敗しました",
				L"MFCreateWaveFormatExFromMFMediaType(outputMediaType.Get(), &waveFormat, &size)",
				L"AudioResource::AudioResource()"
			);


			CopyMemory(&pImpl->m_WaveFormat, waveFormat, sizeof(pImpl->m_WaveFormat));
			CoTaskMemFree(waveFormat);

			PROPVARIANT propVariant;

			ThrowIfFailed(
				reader->GetPresentationAttribute(static_cast<uint32>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &propVariant),
				L"アトリビュートの設定に失敗しました",
				L"reader->GetPresentationAttribute(static_cast<uint32>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &propVariant)",
				L"AudioResource::AudioResource()"
			);

			// 'duration' is in 100ns units; convert to seconds, and round up
			// to the nearest whole byte.
			LONGLONG duration = propVariant.uhVal.QuadPart;
			unsigned int maxStreamLengthInBytes =
				static_cast<unsigned int>(
				((duration * static_cast<ULONGLONG>(pImpl->m_WaveFormat.nAvgBytesPerSec)) + 10000000) /
					10000000
					);

			pImpl->m_SoundData.resize(maxStreamLengthInBytes);

			ComPtr<IMFSample> sample;
			ComPtr<IMFMediaBuffer> mediaBuffer;
			DWORD flags = 0;

			int positionInData = 0;
			bool done = false;
			while (!done)
			{

				ThrowIfFailed(
					reader->ReadSample(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &flags, nullptr, &sample),
					L"サンプラーの読み込みに失敗しました",
					L"reader->ReadSample(static_cast<uint32>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &flags, nullptr, &sample)",
					L"AudioResource::AudioResource()"
				);

				if (sample != nullptr)
				{
					ThrowIfFailed(
						sample->ConvertToContiguousBuffer(&mediaBuffer),
						L"サンプラーのコンバートに失敗しました",
						L"sample->ConvertToContiguousBuffer(&mediaBuffer)",
						L"AudioResource::AudioResource()"
					);

					BYTE *audioData = nullptr;
					DWORD sampleBufferLength = 0;

					ThrowIfFailed(
						mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength),
						L"バッファのLockに失敗しました",
						L"mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength)",
						L"AudioResource::AudioResource()"
					);

					for (DWORD i = 0; i < sampleBufferLength; i++)
					{
						pImpl->m_SoundData[positionInData++] = audioData[i];
					}
				}
				if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
				{
					done = true;
				}
			}
		}
		catch (...) {
			throw;
		}
	}
	AudioResource::~AudioResource() {}

	const vector<byte>& AudioResource::GetSoundData()const {
		return pImpl->m_SoundData;
	}

	WAVEFORMATEX*  AudioResource::GetOutputWaveFormatEx()const {
		return &pImpl->m_WaveFormat;
	}



	//--------------------------------------------------------------------------------------
	//	struct EventDispatcher::Impl;
	//	用途: Impl構造体
	//--------------------------------------------------------------------------------------
	struct EventDispatcher::Impl {
		//イベントのキュー
		list< shared_ptr<Event> > m_PriorityQ;
		map<wstring, vector<weak_ptr<ObjectInterface>>> m_EventInterfaceGroupMap;
		//
		//--------------------------------------------------------------------------------------
		//	void Discharge(
		//	const Event& event	//イベント
		//	);
		//用途: イベントの送信
		//戻り値: なし
		//--------------------------------------------------------------------------------------
		void Discharge(const shared_ptr<Event>& event);
		Impl() {}
		~Impl() {}
	};

	void EventDispatcher::Impl::Discharge(const shared_ptr<Event>& event) {
		auto shptr = event->m_Receiver.lock();
		if (shptr) {
			//受け手が有効
			shptr->OnEvent(event);
		}
	}



	//--------------------------------------------------------------------------------------
	///	イベント配送クラス
	//--------------------------------------------------------------------------------------
	//構築と破棄
	EventDispatcher::EventDispatcher():
		pImpl(new Impl())
	{}
	EventDispatcher::~EventDispatcher() {}

	void EventDispatcher::AddEventReceiverGroup(const wstring& GroupKey, const shared_ptr<ObjectInterface>& Receiver) {
		auto it = pImpl->m_EventInterfaceGroupMap.find(GroupKey);
		if (it != pImpl->m_EventInterfaceGroupMap.end()) {
			//キーがあった
			it->second.push_back(Receiver);
		}
		else {
			//グループがない
			vector<weak_ptr<ObjectInterface>> vec;
			pImpl->m_EventInterfaceGroupMap[GroupKey] = vec;
			pImpl->m_EventInterfaceGroupMap[GroupKey].push_back(Receiver);
		}
	}


	//イベントのPOST（キューに入れる）
	void EventDispatcher::PostEvent(float Delay, const shared_ptr<ObjectInterface>& Sender, const shared_ptr<ObjectInterface>& Receiver,
		const wstring& MsgStr, shared_ptr<void>& Info) {
		//イベントの作成 
		auto Ptr = make_shared<Event>(Delay, Sender, Receiver, MsgStr, Info);
		//キューにためる
		pImpl->m_PriorityQ.push_back(Ptr);
	}

	void EventDispatcher::PostEvent(float DispatchTime, const shared_ptr<ObjectInterface>& Sender, const wstring& ReceiverKey,
		const wstring& MsgStr, shared_ptr<void>& Info) {
		//ReceiverKeyによる相手の特定
		//重複キーの検査
		auto it = pImpl->m_EventInterfaceGroupMap.find(ReceiverKey);
		if (it != pImpl->m_EventInterfaceGroupMap.end()) {
			//キーがあった
			for (auto v : it->second) {
				auto shptr = v.lock();
				if (shptr) {
					//イベントの作成 
					auto Ptr = make_shared<Event>(0.0f, Sender, shptr, MsgStr, Info);
					//キューにためる
					pImpl->m_PriorityQ.push_back(Ptr);
				}
			}
		}
		//キーが見つからなくても何もしない
	}


	//イベントのSEND（キューに入れずにそのまま送る）
	void EventDispatcher::SendEvent(const shared_ptr<ObjectInterface>& Sender, const shared_ptr<ObjectInterface>& Receiver,
		const wstring& MsgStr, shared_ptr<void>& Info) {
		//イベントの作成 
		auto Ptr = make_shared<Event>(0.0f, Sender, Receiver, MsgStr, Info);
		//送信
		pImpl->Discharge(Ptr);
	}

	void EventDispatcher::SendEvent(const shared_ptr<ObjectInterface>& Sender, const wstring& ReceiverKey,
		const wstring& MsgStr, shared_ptr<void>& Info) {
		//ReceiverKeyによる相手の特定
		//重複キーの検査
		auto it = pImpl->m_EventInterfaceGroupMap.find(ReceiverKey);
		if (it != pImpl->m_EventInterfaceGroupMap.end()) {
			//キーがあった
			for (auto v : it->second) {
				auto shptr = v.lock();
				if (shptr) {
					//イベントの作成 
					auto Ptr = make_shared<Event>(0.0f, Sender, shptr, MsgStr, Info);
					//イベントの送出
					pImpl->Discharge(Ptr);
				}
			}
		}
		//キーが見つからなくても何もしない
	}


	void EventDispatcher::DispatchDelayedEvwnt() {
		//前回のターンからの時間
		float ElapsedTime = App::GetApp()->GetElapsedTime();
		auto it = pImpl->m_PriorityQ.begin();
		while (it != pImpl->m_PriorityQ.end()) {
			(*it)->m_DispatchTime -= ElapsedTime;
			if ((*it)->m_DispatchTime <= 0.0f) {
				(*it)->m_DispatchTime = 0.0f;
				//メッセージの送信
				pImpl->Discharge(*it);
				//キューから削除
				it = pImpl->m_PriorityQ.erase(it);
				//削除後のイテレータが「最後」の
				//ときはループを抜ける
				if (it == pImpl->m_PriorityQ.end()) {
					break;
				}
			}
			else {
				it++;
			}
		}
	}

	void EventDispatcher::ClearEventQ() {
		pImpl->m_PriorityQ.clear();
	}





	//--------------------------------------------------------------------------------------
	//	class App;
	//	用途: アプリケーションクラス
	//--------------------------------------------------------------------------------------
	//static変数実体
	unique_ptr<App, App::AppDeleter> App::m_App;
	//構築
	App::App(HINSTANCE hInstance, HWND hWnd, bool FullScreen, UINT Width, UINT Height) :
		m_hInstance{ hInstance },
		m_hWnd{ hWnd },
		m_FullScreen{ FullScreen },
		m_GameWidth{ Width },
		m_GameHeight{ Height },
		m_Timer(),
		m_ScriptsDirActive(false)
	{
		try {
			//基準ディレクトリの設定
			//相対パスにすると、ファイルダイアログでカレントパスが狂うので
			//絶対パス指定
			wchar_t Modulebuff[MAX_PATH];
			wchar_t Drivebuff[_MAX_DRIVE];
			wchar_t Dirbuff[_MAX_DIR];
			wchar_t FileNamebuff[_MAX_FNAME];
			wchar_t Extbuff[_MAX_EXT];

			::ZeroMemory(Modulebuff, sizeof(Modulebuff));
			::ZeroMemory(Drivebuff, sizeof(Drivebuff));
			::ZeroMemory(Dirbuff, sizeof(Dirbuff));
			::ZeroMemory(FileNamebuff, sizeof(FileNamebuff));
			::ZeroMemory(Extbuff, sizeof(Extbuff));

			//モジュール名（プログラムファイル名）を得る
			if (!::GetModuleFileName(nullptr, Modulebuff, sizeof(Modulebuff))) {
				throw BaseException(
					L"モジュールが取得できません。",
					L"if(!::GetModuleFileName())",
					L"App::App()"
				);
			}
			m_wstrModulePath = Modulebuff;
			//モジュール名から、各ブロックに分ける
			_wsplitpath_s(Modulebuff,
				Drivebuff, _MAX_DRIVE,
				Dirbuff, _MAX_DIR,
				FileNamebuff, _MAX_FNAME,
				Extbuff, _MAX_EXT);

			//ドライブ名の取得
			m_wstrDir = Drivebuff;
			//ディレクトリ名の取得
			m_wstrDir += Dirbuff;
			//mediaディレクトリを探す
			m_wstrDataPath = m_wstrDir;
			m_wstrDataPath += L"media";
			//まず、実行ファイルと同じディレクトリを探す
			DWORD RetCode;
			RetCode = GetFileAttributes(m_wstrDataPath.c_str());
			if (RetCode == 0xFFFFFFFF) {
				//失敗した
				m_wstrDataPath = m_wstrDir;
				m_wstrDataPath += L"..\\media";
				RetCode = GetFileAttributes(m_wstrDataPath.c_str());
				if (RetCode == 0xFFFFFFFF) {
					//再び失敗した
					throw BaseException(
						L"mediaディレクトリを確認できません。",
						L"if(RetCode == 0xFFFFFFFF)",
						L"App::App()"
					);
				}
				else {
					m_wstrDataPath += L"\\";
					//相対パスの設定
					m_wstrRelativeDataPath = L"..\\media\\";
				}
			}
			else {
				m_wstrDataPath += L"\\";
				//相対パスの設定
				m_wstrRelativeDataPath = L"media\\";
			}
			m_wstrShadersPath = m_wstrDataPath + L"Shaders\\";
			m_wstrRelativeShadersPath = m_wstrRelativeDataPath + L"Shaders\\";

			//Scriptsディレクトリを探す
			m_wstrScriptsPath = m_wstrDir;
			m_wstrScriptsPath += L"scripts";
			//まず、実行ファイルと同じディレクトリを探す
			RetCode = GetFileAttributes(m_wstrScriptsPath.c_str());
			if (RetCode == 0xFFFFFFFF) {
				//失敗した
				m_wstrScriptsPath = m_wstrDir;
				m_wstrScriptsPath += L"..\\scripts";
				RetCode = GetFileAttributes(m_wstrDataPath.c_str());
				if (RetCode == 0xFFFFFFFF) {
					//再び失敗した
					//Scriptsディレクトリは必須ではないので再び
					//実行ファイルと同じディレクトリに設定
					m_ScriptsDirActive = false;
					m_wstrScriptsPath = L"";
					m_wstrScriptsPath += L"";
				}
				else {
					m_ScriptsDirActive = true;
					m_wstrScriptsPath += L"\\";
					//相対パスの設定
					m_wstrRelativeScriptsPath = L"..\\scripts\\";
				}
			}
			else {
				m_ScriptsDirActive = true;
				m_wstrScriptsPath += L"\\";
				//相対パスの設定
				m_wstrRelativeScriptsPath = L"scripts\\";
			}

			//Assetsディレクトリを探す
			m_wstrRelativeAssetsPath = m_wstrDir;
			m_wstrRelativeAssetsPath += L"..\\..\\Assets";
			//相対ディレクトリを探す
			RetCode = GetFileAttributes(m_wstrRelativeAssetsPath.c_str());
			if (RetCode == 0xFFFFFFFF) {
				//失敗した
				//アセットディレクトリをメディアディレクトリにする
				m_wstrRelativeAssetsPath = m_wstrRelativeDataPath;
			}
			else {
				//成功した
				m_wstrRelativeAssetsPath += L"\\";
			}



			////デバイスリソースの構築
			m_DeviceResources = shared_ptr<DeviceResources>(new DeviceResources(hWnd, FullScreen, Width, Height));
			//オーディオマネージャの取得
			GetAudioManager();
			//イベント配送クラス
			m_EventDispatcher = make_shared<EventDispatcher>();
			//乱数の初期化
			srand((unsigned)time(nullptr));

		}
		catch (...) {
			throw;
		}
	}

	//シングルトン構築
	unique_ptr<App, App::AppDeleter>& App::CreateApp(HINSTANCE hInstance, HWND hWnd,
		bool FullScreen, UINT Width, UINT Height, bool ShadowActive) {
		try {
			if (m_App.get() == 0) {
				//自分自身の構築
				m_App.reset(new App(hInstance, hWnd, FullScreen, Width, Height));
				m_App->AfterInitContents(ShadowActive);

			}
			return m_App;
		}
		catch (...) {
			throw;
		}
	}

	//シングルトンアクセサ
	unique_ptr<App, App::AppDeleter>& App::GetApp() {
		try {
			if (m_App.get() == 0) {
				throw BaseException(
					L"アプリケーションがまだ作成されてません",
					L"if (m_App.get() == 0)",
					L"App::GetApp()"
				);
			}
			return m_App;
		}
		catch (...) {
			throw;
		}

	}

	bool App::AppCheck() {
		if (m_App.get() == 0) {
			return false;
		}
		return true;
	}

	//強制破棄
	void App::DeleteApp() {
		if (m_App.get()) {
			m_App.reset();
		}
	}

	// オーディオマネージャの取得
	unique_ptr<AudioManager>& App::GetAudioManager() {
		try {
			if (m_AudioManager.get() == 0) {
				m_AudioManager.reset(new AudioManager(m_hWnd));
				m_AudioManager->CreateDeviceIndependentResources();

			}
			return m_AudioManager;
		}
		catch (...) {
			throw;
		}
	}

	void App::AfterInitContents(bool ShadowActive) {
		if (!m_DeviceResources) {
			throw BaseException(
				L"デバイスが初期化されていません",
				L"if (!m_DeviceResources)",
				L"App::AfterInitContents()"
			);
		}
		m_DeviceResources->AfterInitContents(ShadowActive);
	}


	void App::RegisterResource(const wstring& Key, const shared_ptr<BaseResource>& ResObj) {
		try {
			if (Key == L"") {
				throw BaseException(
					L"キーが空白です",
					L"if(Key == L\"\")",
					L"App::RegisterResource()"
				);
			}
			if (!ResObj) {
				throw BaseException(
					L"リソースが不定です",
					L"if(!pResObj)",
					L"App::RegisterResource()"
				);
			}
			map<wstring, shared_ptr<BaseResource> >::iterator it;
			//重複ポインタの検査
			for (it = m_ResMap.begin(); it != m_ResMap.end(); it++) {
				if (it->second == ResObj) {
					//ポインタが重複していても、キーが同じなら
					//すでに登録されているのでリターン
					if (it->first == Key) {
						return;
					}
					wstring keyerr = Key;
					throw BaseException(
						L"すでにそのリソースは登録されています",
						keyerr,
						L"App::RegisterResource()"
					);
				}
			}
			//重複キーの検査
			it = m_ResMap.find(Key);
			if (it != m_ResMap.end()) {
				//指定の名前が見つかった
				//例外発生
				wstring keyerr = Key;
				throw BaseException(
					L"すでにそのキーは登録されています",
					keyerr,
					L"App::RegisterResource()"
				);
			}
			else {
				//見つからない
				//リソースペアの追加
				m_ResMap[Key] = ResObj;
			}
		}
		catch (...) {
			throw;
		}
	}

	//テクスチャの登録(同じキーのテクスチャがなければファイル名で作成し、登録)
	//同じ名前のテクスチャがあればそのポインタを返す
	shared_ptr<TextureResource> App::RegisterTexture(const wstring& Key, const wstring& TextureFileName, const wstring& TexType) {
		if (CheckResource<TextureResource>(Key)) {
			return GetResource<TextureResource>(Key);
		}
		//
		auto PtrTexture = TextureResource::CreateTextureResource(TextureFileName, TexType);
		RegisterResource(Key, PtrTexture);
		return PtrTexture;
	}


	//Wavの登録(同じキーのWavがなければファイル名で作成し、登録)
	//同じ名前のWavがあればそのポインタを返す
	shared_ptr<AudioResource> App::RegisterWav(const wstring& Key, const wstring& WavFileName) {
		if (CheckResource<AudioResource>(Key)) {
			return GetResource<AudioResource>(Key);
		}
		//
		auto PtrWav = ObjectFactory::Create<AudioResource>(WavFileName);
		RegisterResource(Key, PtrWav);
		return PtrWav;
	}

	void App::UpdateDraw(unsigned int SyncInterval) {
		if (!m_SceneInterface) {
			//シーンがが無効なら
			throw BaseException(
				L"シーンがありません",
				L"if(!m_SceneInterface)",
				L"App::UpdateDraw()"
			);
		}

		// シーン オブジェクトを更新します。
		m_InputDevice.ResetControlerState();
		m_Timer.Tick([&]()
		{
			//イベントキューの送信
			m_EventDispatcher->DispatchDelayedEvwnt();
			m_SceneInterface->OnUpdate();
		});
		// 初回更新前にレンダリングは行わない。
		if (GetFrameCount() == 0)
		{
			return;
		}
		m_SceneInterface->OnDraw();
		// バックバッファからフロントバッファに転送
		m_DeviceResources->Present(SyncInterval,0);
	}


	void App::OnMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	}





}
//end basecross
