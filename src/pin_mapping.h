#include <cstdint>
#include "common_def.h"

// static std::array<std::array<VBtn,4>, 42>  PinMapping = {};
// static std::array<int, 42>                 PinMapped;
// static int                                 PinMappedCount = 0;

// static void setupPinMappingTable_nologo() // 无名科技
// {
//     mapButton(39, VBtn::BACK);
//     mapButton(41, VBtn::START);
//     mapButton(4, VBtn::HOME);

//     mapButton(13, VBtn::UP);
//     mapButton(14, VBtn::DOWN);
//     mapButton(15, VBtn::RIGHT);
//     mapButton(16, VBtn::LEFT);

//     mapButton(17, VBtn::A);
//     mapButton(18, VBtn::B);
//     mapButton(33, VBtn::LB);
//     mapButton(34, VBtn::RB);
//     // // //
//     mapButton(35, VBtn::X);
//     mapButton(36, VBtn::Y);
//     mapButton(37, VBtn::LT);
//     mapButton(38, VBtn::RT);

//     mapButton(1, VBtn::LS);
//     mapButton(2, VBtn::RS);

//     // mapButton(4, VBtn::A, VBtn::X);
//     mapButton(5, VBtn::B, VBtn::Y);
//     mapButton(6, VBtn::RT, VBtn::RB);
//     mapButton(7, VBtn::A, VBtn::B, VBtn::RT);
//     mapButton(8, VBtn::X, VBtn::Y, VBtn::RB);
//     mapButton(9, VBtn::A, VBtn::B, VBtn::LT, VBtn::LB);
//     mapButton(10, VBtn::A, VBtn::B, VBtn::X, VBtn::Y);
// }

// static void setupPinMappingTable_walnut() // 核桃派
// {
//     mapButton(21, VBtn::A);
//     mapButton(34, VBtn::B);
//     mapButton(35, VBtn::LT);
//     mapButton(36, VBtn::RT);
//     //
//     mapButton(37, VBtn::X);
//     mapButton(38, VBtn::Y);
//     mapButton(39, VBtn::LB);
//     mapButton(40, VBtn::RB);

//     mapButton(41, VBtn::BACK);
//     mapButton(2, VBtn::START);
//     mapButton(6, VBtn::HOME);

//     mapButton(4, VBtn::LS);
//     mapButton(5, VBtn::RS);

//     mapButton(15, VBtn::UP);
//     mapButton(16, VBtn::DOWN);
//     mapButton(17, VBtn::RIGHT);
//     mapButton(18, VBtn::LEFT);
// }

// static void setupPinMappingTable_devkitc() // devkit
// {
//     mapButton(4, VBtn::A);
//     mapButton(5, VBtn::B);
//     mapButton(6, VBtn::X);
//     mapButton(7, VBtn::Y);

//     mapButton(15, VBtn::LB);
//     mapButton(16, VBtn::RB);
//     mapButton(17, VBtn::LS);
//     mapButton(18, VBtn::RS);

//     mapButton(3, VBtn::BACK);
//     mapButton(8, VBtn::START);
//     mapButton(9, VBtn::HOME);

//     mapButton(11, VBtn::UP);
//     mapButton(12, VBtn::DOWN);
//     mapButton(13, VBtn::RIGHT);
//     mapButton(14, VBtn::LEFT);

//     mapButton(1, VBtn::LT);
//     mapButton(2, VBtn::RT);


//     mapButton(41, VBtn::A, VBtn::X);
//     mapButton(40, VBtn::B, VBtn::Y);
//     mapButton(39, VBtn::RT, VBtn::RB);
//     mapButton(38, VBtn::A, VBtn::B, VBtn::RT);
//     mapButton(37, VBtn::X, VBtn::Y, VBtn::RB);
//     mapButton(36, VBtn::A, VBtn::B, VBtn::LT, VBtn::LB);
//     mapButton(35, VBtn::A, VBtn::B, VBtn::X, VBtn::Y);
// }