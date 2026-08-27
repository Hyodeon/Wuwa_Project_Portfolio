# Architecture and Guidelines

## 2. GAS Migration 원칙 (ActionState -> GAS)
현재 C++ 열거형(`EActionState`)으로 하드코딩된 상태 관리를 100% GAS Tag 기반으로 전환 중입니다.
1. **입력 바인딩 구조**: 캐릭터 클래스는 입력 이벤트 감지 및 GA 발동만을 처리합니다.
2. **GA의 독립성 보장**: 콤보 관리(`GA_Attack`), 회피 로직(`GA_Dodge`) 등은 각각의 어빌리티 내부에서 몽타주와 함께 자체 처리. 공간/타겟팅 수학 계산은 캐릭터 C++ 헬퍼 함수를 참조합니다.
3. **상태 관리 일원화**: `ActionState` 변수를 퇴역시키고, `Ability.State.Attacking`, `State.Invulnerable` 등의 Gameplay Tag로 상태를 정의하며, CancelAbilitiesWithTag로 어빌리티 간 우선순위를 제어합니다.
4. **퍼펙트 회피 연동**: 적 공격 직전 `State.DodgeWindow` 태그 존재 시 퍼펙트 회피 성공 이벤트(`Event.JustDodge.Success`) 발생 및 대미지 무효화.
5. **보수적 접근 원칙 (Blueprint Protection)**: `EActionState` 중 블루프린트에서 사용 중인 열거형은 `UPROPERTY`를 떼지 않고 유지하며, C++ 내부 로직에서만 GAS 태그를 사용하도록 우회(Bypass)합니다.
6. **수학적 계산 분리 원칙**: 회피 방향 계산, 타겟팅 등 수학적 공간적 연산은 캐릭터(C++) 내부에 헬퍼 함수(`CalculateDodgeSectionAndWarp` 등)로 남겨두고, GA에서는 이를 호출하여 리턴값만 받아 사용합니다.
7. **모션 워핑 거리 클램핑**: 록온 상태에서 적을 향해 전진(Front) 회피할 경우, 적을 뚫고 지나가는 현상을 막기 위해 캐릭터 반경(`MinStandDistance`)을 고려하여 워핑 거리를 클램프합니다.

## 3. 핵심 전투 행동 정의 및 우선순위 규칙 (Core Combat Behaviors)
시스템이 점차 고도화됨에 따라, 게임 내 주요 행동들의 "발동 조건", "태그(Tag) 생명주기", 그리고 "캔슬 우선순위"를 아래와 같이 엄격하게 정의합니다.

### 1) 기본 공격 (`GA_Attack`)
- **발동 조건:** 대기 중이거나 이동 진행 중일 때 클릭. (단, 피격이나 퍼펙트 회피 반격기 중일 때는 발동 불가)
- **콤보 시스템:** `Event.Combat.AttackInput`을 통해 입력을 버퍼링하고, 애니메이션의 `CheckCombo` 노티파이 타이밍에 버퍼링된 입력이 있다면 다음 모션으로 이어집니다.
- **캔슬:** 회피(Dodge)를 통해서만 캔슬될 수 있습니다.
- **상태 관리:** 진행 중에는 `State.Attack` 태그를 부착합니다 (의도된 설계). 모션 워핑(루트 모션)으로 신체를 이동을 통제합니다. 종료 시 무기 콜리전을 안전하게 끕니다.

### 2) 회피 / 대시 (`GA_Dodge`)
- **발동 조건:** 쿨타임이 없고 스태미너 제약 이외에는 거의 언제든 발동 가능.
- **캔슬 계층 최상위:** 기본 공격(`Ability.Attack`) 등을 즉시 캔슬(CancelAbilitiesWithTag)하고 자신을 발동시킬 수 있습니다.
- **타겟팅:** 록온 시에는 록온 타겟을 기준으로 방향을 결정하며, 미시에는 입력 방향으로 회전합니다.

### 3) 퍼펙트 회피 (`GA_JustDodge`)
- **발동 조건:** 적 공격 특정 모션 직전에 존재하는 `State.DodgeWindow` 구간에 적의 공격에 피격받을 시(`Event.JustDodge.Success`) 수동 발동합니다.
- **캔슬 및 무적:** 일반 회피 대시 모션을 즉시 캔슬하며, 1.0초간 글로벌 시간 왜곡(Time Dilation)과 무적(`State.Invulnerable`)을 부여합니다.
- **입력 버퍼링:** 진행되는 동안 평타처럼 좌클릭 이벤트를 버퍼링하여, 애니메이션의 노티파이 타이밍에 반격기로 이어질 수 있습니다.

### 4) 회피 반격기 (`GA_DodgeCounterAttack`)
- **발동 조건:** 퍼펙트 회피 동작 도중 버퍼링된 클릭 입력이 존재할 때, 퍼펙트 회피(`GA_JustDodge`) 로직에 의해 *자동으로* 호출됩니다. (블루프린트에서 불리지 않음)
- **상태 연장 설계:** 반격기는 독자적인 매크로 상태가 아니라 **"퍼펙트 회피의 연장선"**으로 취급됩니다. 발동 중에는 무적 상태와 함께 **`State.JustDodge.Active`** 태그를 스스로에게 부여(Owned)합니다.
- **결과:** 해당 태그 유지 덕분에, 반격기 도중 평타가 덧씌워지지 않고, 이동/점프 등도 퍼펙트 회피와 동일하게 차단됩니다. 종료 시 무기 콜리전을 끄고 태그를 제거합니다.

## 4. AI 작업 규칙 (Guidelines for AI)
AI 어시스턴트(Antigravity 등)가 본 프로젝트에서 코드를 생성하거나 리팩토링할 때 반드시 준수해야 할 규칙입니다.

1. **[CRITICAL] 절대적인 UTF-8 인코딩 엄수**: 
   - **모든 문서 및 코드 파일에 대한 읽기/쓰기 작업 시, 반드시 `UTF-8` 인코딩을 명시해야 합니다.**
   - PowerShell(`Get-Content`, `Set-Content` 등) 환경에서 임의로 인코딩을 변환하여 한글 바이트가 깨지는(`?` 문자로 치환되는) 현상을 절대적으로 방지해야 합니다. 가급적 기본 툴(`replace_file_content`)을 우선 사용하세요.
2. **토큰 덤프 방지 (No Full-Script Dumps)**: 
   - 스크립트 전체를 덤프하거나 불필요한 보일러플레이트를 반복 출력하지 마세요.
   - 부분 수정이 필요한 경우 `replace_file_content` 또는 `multi_replace_file_content`를 사용하여 **변경된 블록(Chunk)만 수정**하세요.
3. **스텝 단위(Step-by-Step) 진행**: 
   - 한 번에 하나의 도메인에 집중하여 작업하세요.
   - 작업 완료 후 반드시 어떤 부분이 수정되었는지 요약하고, 다음 스텝 진행 여부를 물어보세요.
4. **언리얼 AI (BT) 개발 필수 원칙**:
   - **포커스 우선순위**: 이동 컴포넌트(MoveTo)가 시선을 뺏지 못하게 하려면, C++에서 `EAIFocusPriority::Gameplay` 등급으로 Focus를 강제 고정해야 합니다.
   - **토큰 누수 방지**: Combat Token을 할당받은 AI는 공격 종료 시점뿐만 아니라, **피격(GetHit)** 및 **사망(Die)** 시에도 무조건 Token을 반납(Release)해야 합니다.
   - **BT 분기 논리**: 하위 조건을 나열할 때 Sequence(AND)와 Selector(OR)를 명확히 구분하여, 조건 불만족 시 트리 전체가 실패(Freeze)하는 것을 방지하세요.
   - **우회 공격(Raycast)**: 공격(Slam) 전 Raycast를 쏘아 아군이 앞을 막고 있다면 공격 조건을 해제하고, 자연스럽게 스트레이핑(Strafe)으로 넘어가 우회하도록 설계해야 합니다.

5. **전투 AI 상세 기록 참조**: 스웜(군집) AI 트러블슈팅, 토큰 누수, 시선 고정 버그 등 구체적인 전투 시스템 이슈와 해결 과정은 같은 폴더의 SwarmAI_Troubleshooting_Report.md 문서를 반드시 참조하세요.

## 5. Current Context (현재 작업 위치)
- **현재 진행 중인 작업:** `ActionState` 기반 전투 시스템을 순수 GAS로 전면 마이그레이션.
- **종료된 단계 (Step 1~4)**: 
  - `WuwaGameplayTags` 정립 완료.
  - 블루프린트를 깨지 않는 보수적 접근으로 `AttackEnd`, `DodgeEnd` 우회 완료 및 `GA_Dodge` 캔슬 윈도우 연동.
  - 모션/방향 계산 로직을 캐릭터의 `CalculateDodgeSectionAndWarp` 함수로 분리 완료.
  - 퍼펙트 회피와 반격기를 2개의 어빌리티(`GA_JustDodge`, `GA_DodgeCounterAttack`)로 분리 설계 완료.
  - 평타 콤보의 입력 버퍼링과 노티파이 기반 타이밍 완벽 구현 (`Event.Combat.AttackInput`, `CheckCombo`).

- **종료된 단계 (Step 5): 스웜(군집) AI 및 전투 토큰 시스템 구축 완수**
  - **개요:** 액션 RPG 특유의 완벽한 핑퐁 전투 AI 구현 완료.
  - **전투 토큰 매니저 (`UCombatManagerComponent`):** 플레이어에게 부착되어 적들에게 '근접 공격(Melee Token)', '도약 공격(Support Token)' 발급 통제.
  - **동적 호전성 (Aggressiveness):** 체력과 최근 피격 횟수 기반으로 호전성 조절 (회피 후 우회 공격 등).
  - **전략적 포지셔닝 (Smart Strafing & Raycast):** 아군 충돌 방지 회피 기동 및 플레이어 주변을 관통하지 않고 공전하는 스트레이핑 수리 완료. 
  - **글로벌 쿨타임 (Global Cooldown):** 스킬과 일반 공격 간의 연계를 제한하는 C++ 기반 `BTDecorator_SharedCooldown` 적용 완료.

- **다음 단계 (Step 6): 아머(Armor) 및 공진(Resonance) UI 시스템 구축** (자세한 기획은 Implementation_plan.md 참조)
