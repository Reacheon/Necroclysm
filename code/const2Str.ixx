export module const2Str;

import std;
import constVar;
import globalVar;

export std::wstring itemCategory2String(itemCategory input)
{
	if (input == itemCategory::equipment) return sysStr[41]; // 장비
	else if (input == itemCategory::foods) return sysStr[42]; // 음식
	else if (input == itemCategory::tools) return sysStr[43]; // 도구
	else if (input == itemCategory::tech) return sysStr[44]; // 기술
	else if (input == itemCategory::consumables) return sysStr[45]; // 소모품
	else if (input == itemCategory::vehicles) return sysStr[46]; // 차량
	else if (input == itemCategory::structures) return sysStr[47]; // 건축
	else if (input == itemCategory::materials) return sysStr[48]; // 재료
	else return sysStr[204]; // 미확인 카테고리
};

export std::wstring itemSubcategory2String(itemSubcategory input)
{
	// 장비
	if (input == itemSubcategory::equipment_melee) return sysStr[158]; // 근접무기
	else if (input == itemSubcategory::equipment_ranged) return sysStr[159]; // 원거리무기
	else if (input == itemSubcategory::equipment_firearms) return sysStr[160]; // 화기
	else if (input == itemSubcategory::equipment_throwing) return sysStr[161]; // 투척무기
	else if (input == itemSubcategory::equipment_clothing) return sysStr[162]; // 의류

	// 음식
	else if (input == itemSubcategory::foods_cooked) return sysStr[163]; // 요리
	else if (input == itemSubcategory::foods_processed) return sysStr[164]; // 가공식품
	else if (input == itemSubcategory::foods_preserved) return sysStr[165]; // 보존식품
	else if (input == itemSubcategory::foods_drinks) return sysStr[166]; // 음료
	else if (input == itemSubcategory::foods_ingredients) return sysStr[167]; // 재료

	// 도구
	else if (input == itemSubcategory::tools_hand) return sysStr[168]; // 수공구
	else if (input == itemSubcategory::tools_power) return sysStr[169]; // 동력공구
	else if (input == itemSubcategory::tools_containers) return sysStr[170]; // 용기
	else if (input == itemSubcategory::tools_etc) return sysStr[171]; // 기타

	// 기술
	else if (input == itemSubcategory::tech_bionics) return sysStr[172]; // 바이오닉
	else if (input == itemSubcategory::tech_powerArmor) return sysStr[173]; // 파워아머

	// 소모품
	else if (input == itemSubcategory::consumable_medicine) return sysStr[174]; // 의약품
	else if (input == itemSubcategory::consumable_ammo) return sysStr[175]; // 탄약
	else if (input == itemSubcategory::consumable_fuel) return sysStr[176]; // 연료
	else if (input == itemSubcategory::consumable_etc) return sysStr[171]; // 기타

	// 차량
	else if (input == itemSubcategory::vehicle_frames) return sysStr[177]; // 프레임
	else if (input == itemSubcategory::vehicle_power) return sysStr[178]; // 동력
	else if (input == itemSubcategory::vehicle_exteriors) return sysStr[179]; // 외장
	else if (input == itemSubcategory::vehicle_parts) return sysStr[180]; // 부품


	// 구조물
	else if (input == itemSubcategory::structure_walls) return sysStr[181]; // 벽
	else if (input == itemSubcategory::structure_floors) return sysStr[182]; // 바닥
	else if (input == itemSubcategory::structure_props) return sysStr[183]; // 설치물

	// 재료
	else if (input == itemSubcategory::material_metals) return sysStr[184]; // 금속
	else if (input == itemSubcategory::material_organic) return sysStr[185]; // 유기물
	else if (input == itemSubcategory::material_components) return sysStr[186]; // 부품
	else if (input == itemSubcategory::material_chemicals) return sysStr[187]; // 화학
	else if (input == itemSubcategory::material_etc) return sysStr[171]; // 기타

	else return sysStr[205];//ERROR
};

export std::wstring toolQuality2String(int input)
{
	if (input == toolQuality::screwDriving) return sysStr[189];//나사돌리기
	else if (input == toolQuality::drilling) return sysStr[190];//드릴
	else if (input == toolQuality::welding) return sysStr[191];//용접
	else if (input == toolQuality::soldering) return sysStr[192];//땜질
	else if (input == toolQuality::cutting) return sysStr[193];//절단
	else if (input == toolQuality::sawing) return sysStr[194];//톱질
	else if (input == toolQuality::hammering) return sysStr[195];//망치
	else if (input == toolQuality::digging) return sysStr[196];//굴착
	else if (input == toolQuality::sewing) return sysStr[197];//바느질
	else if (input == toolQuality::distillation) return sysStr[198];//증류
	else if (input == toolQuality::boiling) return sysStr[199];//끓이기
	else if (input == toolQuality::frying) return sysStr[200];//튀기기
	else if (input == toolQuality::roasting) return sysStr[201];//굽기
	else if (input == toolQuality::boltTurning) return sysStr[202];//볼트돌리기
	else return sysStr[203];//미확인 기술
};

