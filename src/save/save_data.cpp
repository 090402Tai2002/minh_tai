#include "lib.h"
namespace main
{

    namespace save_data
    {
        
        Preferences preferences;
        void saveData()
        {
            preferences.begin("config", false);

            // Lưu các giá trị cấu hình vào bộ nhớ
            preferences.putUInt("mat_do", display::mat_do);
            // preferences.putUInt("so_lan_phun", display::so_lan_phun);
            preferences.putUInt("thoi_gian_phun", display::cycle_on);  // chỉnh lại cái này
            preferences.putUInt("thoi_gian_nghi", display::cycle_off); // chỉnh lại cái này
            preferences.putUShort("so_can_cali", display::so_can_cali);
            preferences.putUShort("van_toc_cao", display::van_toc_cao);
            preferences.putUShort("van_toc_thap", display::van_toc_thap);

            // Lưu struct TIME (time_on và time_off) bằng cách tuần tự hóa thành mảng byte
            preferences.putBytes("time_on", &display::time_on, sizeof(display::TIME));// 
            preferences.putBytes("time_off", &display::time_off, sizeof(display::TIME));//
            preferences.end();

        }
        void loadData()
        {
            // Mở không gian lưu trữ với chế độ đọc
            preferences.begin("config", true);
            // Đọc các giá trị từ bộ nhớ, nếu không có giá trị, trả về giá trị mặc định là 0
            display::mat_do = preferences.getUInt("mat_do", 0);
            // display::so_lan_phun = preferences.getUInt("so_lan_phun", 0);
            display::cycle_on = preferences.getUInt("thoi_gian_phun", 0);
            display::cycle_off = preferences.getUInt("thoi_gian_nghi", 0);
            display::so_can_cali = preferences.getUShort("so_can_cali", 0);
            display::van_toc_cao = preferences.getUShort("van_toc_cao", 0);
            display::van_toc_thap = preferences.getUShort("van_toc_thap", 0);

            // Đọc struct TIME từ bộ nhớ
            preferences.getBytes("time_on", &display::time_on, sizeof(display::TIME));
            preferences.getBytes("time_off", &display::time_off, sizeof(display::TIME));
            // Đóng không gian lưu trữ
            preferences.end();
        }

    }

}