export module const2Str;

import std;
import constVar;

export std::wstring itemCategory2String(itemCategory input)
{
	if (input == itemCategory::weapon) return L"무기";
	else if (input == itemCategory::equipment) return L"장비";
	else if (input == itemCategory::tool) return L"도구";
	else if (input == itemCategory::consumable) return L"소모품";
	else if (input == itemCategory::vehicle) return L"차량";
	else if (input == itemCategory::bionic) return L"바이오닉";
	else if (input == itemCategory::structure) return L"구조물";
	else if (input == itemCategory::material) return L"재료";
	else return L"알 수 없는 카테고리";
};

export std::wstring itemSubcategory2String(itemSubcategory input)
{
	// 무기
	if (input == itemSubcategory::weapon_piercing) return L"관통";
	else if (input == itemSubcategory::weapon_cutting) return L"절단";
	else if (input == itemSubcategory::weapon_bashing) return L"타격";
	else if (input == itemSubcategory::weapon_shooting) return L"사격";
	else if (input == itemSubcategory::weapon_throwing) return L"투척";

	// 장비
	else if (input == itemSubcategory::equipment_clothing) return L"의류";
	else if (input == itemSubcategory::equipment_hat) return L"모자";
	else if (input == itemSubcategory::equipment_gloves) return L"장갑";
	else if (input == itemSubcategory::equipment_shoes) return L"신발";
	else if (input == itemSubcategory::equipment_accessory) return L"액세서리";

	// 도구
	else if (input == itemSubcategory::tool_hand) return L"수공구";
	else if (input == itemSubcategory::tool_power) return L"동력공구";
	else if (input == itemSubcategory::tool_container) return L"용기";
	else if (input == itemSubcategory::tool_device) return L"장치";
	else if (input == itemSubcategory::tool_document) return L"문서";
	else if (input == itemSubcategory::tool_etc) return L"기타";

	// 소모품
	else if (input == itemSubcategory::consumable_food) return L"음식";
	else if (input == itemSubcategory::consumable_medicine) return L"의약품";
	else if (input == itemSubcategory::consumable_ammo) return L"탄약";
	else if (input == itemSubcategory::consumable_fuel) return L"연료";
	else if (input == itemSubcategory::consumable_etc) return L"기타";

	// 차량
	else if (input == itemSubcategory::vehicle_frame) return L"프레임";
	else if (input == itemSubcategory::vehicle_engine) return L"엔진";
	else if (input == itemSubcategory::vehicle_exterior) return L"외장";
	else if (input == itemSubcategory::vehicle_transport) return L"수송";
	else if (input == itemSubcategory::vehicle_energy) return L"에너지";
	else if (input == itemSubcategory::vehicle_device) return L"장치";

	// 바이오닉
	else if (input == itemSubcategory::bionic_core) return L"코어";
	else if (input == itemSubcategory::bionic_active) return L"발동계";
	else if (input == itemSubcategory::bionic_passive) return L"지속계";
	else if (input == itemSubcategory::bionic_toggle) return L"전환계";
	else if (input == itemSubcategory::bionic_generator) return L"생산계";
	else if (input == itemSubcategory::bionic_storage) return L"저장계";

	// 구조물
	else if (input == itemSubcategory::structure_wall) return L"벽";
	else if (input == itemSubcategory::structure_floor) return L"바닥";
	else if (input == itemSubcategory::structure_ceil) return L"천장";
	else if (input == itemSubcategory::structure_prop) return L"설치물";
	else if (input == itemSubcategory::structure_electric) return L"전기";
	else if (input == itemSubcategory::structure_pneumatic) return L"유압";

	// 재료
	else if (input == itemSubcategory::material_chemical) return L"화학";
	else if (input == itemSubcategory::material_biological) return L"생물";
	else if (input == itemSubcategory::material_mechanical) return L"기계";
	else if (input == itemSubcategory::material_electrical) return L"전기";
	else if (input == itemSubcategory::material_etc) return L"기타";


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
	else if (input == proficFlag::architecture) return L"건축";
	else return L"미확인 재능";
};
