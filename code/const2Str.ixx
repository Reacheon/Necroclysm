export module const2Str;

import std;
import globalVar;
import constVar;

export std::wstring itemCategory2String(itemCategory input)
{
	if (input == itemCategory::equipment) return sysStr[77]; // 장비
	else if (input == itemCategory::foods) return sysStr[78]; // 음식
	else if (input == itemCategory::tools) return sysStr[79]; // 도구
	else if (input == itemCategory::tech) return sysStr[80]; // 기술
	else if (input == itemCategory::consumables) return sysStr[81]; // 소모품
	else if (input == itemCategory::vehicles) return sysStr[82]; // 차량
	else if (input == itemCategory::structures) return sysStr[83]; // 건축
	else if (input == itemCategory::materials) return sysStr[84]; // 재료
	else return sysStr[292]; // 미확인 카테고리
};

export std::wstring itemSubcategory2String(itemSubcategory input)
{
	// 장비
	if (input == itemSubcategory::equipment_melee) return sysStr[241]; // 근접무기
	else if (input == itemSubcategory::equipment_ranged) return sysStr[242]; // 원거리무기
	else if (input == itemSubcategory::equipment_firearms) return sysStr[243]; // 화기
	else if (input == itemSubcategory::equipment_throwing) return sysStr[244]; // 투척무기
	else if (input == itemSubcategory::equipment_clothing) return sysStr[245]; // 의류

	// 음식
	else if (input == itemSubcategory::foods_cooked) return sysStr[246]; // 요리
	else if (input == itemSubcategory::foods_processed) return sysStr[247]; // 가공식품
	else if (input == itemSubcategory::foods_preserved) return sysStr[248]; // 보존식품
	else if (input == itemSubcategory::foods_drinks) return sysStr[249]; // 음료
	else if (input == itemSubcategory::foods_ingredients) return sysStr[250]; // 재료

	// 도구
	else if (input == itemSubcategory::tools_hand) return sysStr[251]; // 수공구
	else if (input == itemSubcategory::tools_power) return sysStr[252]; // 동력공구
	else if (input == itemSubcategory::tools_containers) return sysStr[253]; // 용기
	else if (input == itemSubcategory::tools_etc) return sysStr[254]; // 기타

	// 기술
	else if (input == itemSubcategory::tech_bionics) return sysStr[255]; // 바이오닉
	else if (input == itemSubcategory::tech_powerArmor) return sysStr[256]; // 파워아머

	// 소모품
	else if (input == itemSubcategory::consumable_medicine) return sysStr[257]; // 의약품
	else if (input == itemSubcategory::consumable_ammo) return sysStr[258]; // 탄약
	else if (input == itemSubcategory::consumable_fuel) return sysStr[259]; // 연료
	else if (input == itemSubcategory::consumable_etc) return sysStr[254]; // 기타

	// 차량
	else if (input == itemSubcategory::vehicle_frame) return sysStr[260]; // 프레임
	else if (input == itemSubcategory::vehicle_engine) return sysStr[261]; // 엔진
	else if (input == itemSubcategory::vehicle_exterior) return sysStr[262]; // 외장
	else if (input == itemSubcategory::vehicle_transport) return sysStr[263]; // 수송
	else if (input == itemSubcategory::vehicle_energy) return sysStr[264]; // 에너지
	else if (input == itemSubcategory::vehicle_device) return sysStr[265]; // 장치

	// 구조물
	else if (input == itemSubcategory::structure_wall) return sysStr[266]; // 벽
	else if (input == itemSubcategory::structure_floor) return sysStr[267]; // 바닥
	else if (input == itemSubcategory::structure_ceil) return sysStr[268]; // 천장
	else if (input == itemSubcategory::structure_prop) return sysStr[269]; // 설치물
	else if (input == itemSubcategory::structure_electric) return sysStr[270]; // 전기
	else if (input == itemSubcategory::structure_pneumatic) return sysStr[271]; // 유압

	// 재료
	else if (input == itemSubcategory::material_metals) return sysStr[272]; // 금속
	else if (input == itemSubcategory::material_organic) return sysStr[273]; // 유기물
	else if (input == itemSubcategory::material_components) return sysStr[274]; // 부품
	else if (input == itemSubcategory::material_chemicals) return sysStr[275]; // 화학
	else if (input == itemSubcategory::material_etc) return sysStr[254]; // 기타

	else return sysStr[294];//ERROR
};

export std::wstring toolQuality2String(int input)
{
	if (input == toolQuality::screwDriving) return sysStr[277];//나사돌리기
	else if (input == toolQuality::drilling) return sysStr[278];//드릴
	else if (input == toolQuality::welding) return sysStr[279];//용접
	else if (input == toolQuality::soldering) return sysStr[280];//땜질
	else if (input == toolQuality::cutting) return sysStr[281];//절단
	else if (input == toolQuality::sawing) return sysStr[282];//톱질
	else if (input == toolQuality::hammering) return sysStr[283];//망치
	else if (input == toolQuality::digging) return sysStr[284];//굴착
	else if (input == toolQuality::sewing) return sysStr[285];//바느질
	else if (input == toolQuality::distillation) return sysStr[286];//증류
	else if (input == toolQuality::boiling) return sysStr[287];//끓이기
	else if (input == toolQuality::frying) return sysStr[288];//튀기기
	else if (input == toolQuality::roasting) return sysStr[289];//굽기
	else if (input == toolQuality::boltTurning) return sysStr[290];//볼트돌리기
	else return sysStr[291];//미확인 기술
};

export std::wstring profic2String(int input)
{
	if (input == proficFlag::fighting) return sysStr[55];//전투
	else if (input == proficFlag::dodging) return sysStr[56];//회피 
	else if (input == proficFlag::stealth) return sysStr[57];//은신
	else if (input == proficFlag::throwing) return sysStr[58];//투척
	else if (input == proficFlag::unarmedCombat) return sysStr[59];//격투
	else if (input == proficFlag::piercingWeapon) return sysStr[60];//관통
	else if (input == proficFlag::cuttingWeapon) return sysStr[61];//절단
	else if (input == proficFlag::bashingWeapon) return sysStr[62];//타격
	else if (input == proficFlag::archery) return sysStr[63];//궁술
	else if (input == proficFlag::gun) return sysStr[64];//총기
	else if (input == proficFlag::electronics) return sysStr[65];//전자공학
	else if (input == proficFlag::chemistry) return sysStr[66];//화학공학
	else if (input == proficFlag::mechanics) return sysStr[67];//기계공학
	else if (input == proficFlag::computer) return sysStr[68];//컴퓨터
	else if (input == proficFlag::medicine) return sysStr[69];//의학
	else if (input == proficFlag::cooking) return sysStr[70];//요리
	else if (input == proficFlag::fabrication) return sysStr[71];//재봉
	else if (input == proficFlag::social) return sysStr[72];//화술
	else if (input == proficFlag::architecture) return sysStr[73];//건축공학
    else return sysStr[293]; // 미확인 숙련도
};