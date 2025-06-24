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
	else return L"알 수 없는 카테고리";
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

	else return L"ERROR";
};

export std::wstring toolQuality2String(int input)
{
	if (input == toolQuality::screwDriving) return L"나사돌리기";
	else if (input == toolQuality::drilling) return L"드릴";
	else if (input == toolQuality::welding) return L"용접";
	else if (input == toolQuality::soldering) return L"땜질";
	else if (input == toolQuality::cutting) return L"절단";
	else if (input == toolQuality::sawing) return L"톱질";
	else if (input == toolQuality::hammering) return L"망치";
	else if (input == toolQuality::digging) return L"굴착";
	else if (input == toolQuality::sewing) return L"바느질";
	else if (input == toolQuality::distillation) return L"증류";
	else if (input == toolQuality::boiling) return L"끓이기";
	else if (input == toolQuality::frying) return L"튀기기";
	else if (input == toolQuality::roasting) return L"굽기";
	else if (input == toolQuality::boltTurning) return L"볼트돌리기";
	else return L"미확인 기술";
};

export std::wstring profic2String(int input)
{
	if (input == proficFlag::fighting) return L"전투";
	else if (input == proficFlag::dodging) return L"회피";
	else if (input == proficFlag::stealth) return L"은신";
	else if (input == proficFlag::throwing) return L"투척";
	else if (input == proficFlag::unarmedCombat) return L"격투";
	else if (input == proficFlag::piercingWeapon) return L"관통";
	else if (input == proficFlag::cuttingWeapon) return L"절단";
	else if (input == proficFlag::bashingWeapon) return L"타격";
	else if (input == proficFlag::archery) return L"궁술";
	else if (input == proficFlag::gun) return L"총기";
	else if (input == proficFlag::electronics) return L"전자공학";
	else if (input == proficFlag::chemistry) return L"화학공학";
	else if (input == proficFlag::mechanics) return L"기계공학";
	else if (input == proficFlag::computer) return L"컴퓨터";
	else if (input == proficFlag::medicine) return L"의학";
	else if (input == proficFlag::cooking) return L"요리";
	else if (input == proficFlag::fabrication) return L"재봉";
	else if (input == proficFlag::social) return L"화술";
	else if (input == proficFlag::architecture) return L"건축공학";
	else return L"미확인 재능";
};