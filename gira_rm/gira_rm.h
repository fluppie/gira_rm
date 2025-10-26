#include "esphome.h"

namespace esphome {
namespace gira_rm {

class GiraRMComponent : public PollingComponent {
 public:
  UARTComponent *uart_{nullptr};
  sensor::Sensor *battery_voltage = new sensor::Sensor();
  sensor::Sensor *temp1 = new sensor::Sensor();
  sensor::Sensor *temp2 = new sensor::Sensor();
  sensor::Sensor *smoke_chamber = new sensor::Sensor();
  sensor::Sensor *hours = new sensor::Sensor();
  sensor::Sensor *alarm_count = new sensor::Sensor();

  GiraRMComponent() = default;
  void set_uart(UARTComponent *uart) { uart_ = uart; }

  void setup() override {
    App.register_sensor(battery_voltage);
    App.register_sensor(temp1);
    App.register_sensor(temp2);
    App.register_sensor(smoke_chamber);
    App.register_sensor(hours);
    App.register_sensor(alarm_count);
  }

  void update() override {
    const uint8_t STX = 0x02, ETX = 0x03;
    const char *cmds[] = {"0464","0868","0969","0B72","0C73","0D74","0E75"};
    std::string responses[7];

    for (int i=0;i<7;i++) {
      std::string frame;
      frame += STX;
      frame += cmds[i];
      frame += ETX;
      uart_->write_str(frame.c_str());
      delay(100);
      std::string buf;
      uint32_t start = millis();
      bool in_frame=false;
      while (millis()-start < 500) {
        if (!uart_->available()) { delay(5); continue; }
        uint8_t b = uart_->read();
        if (b==0x00 || b==0x06) continue;
        if (b==STX) { in_frame=true; buf.clear(); buf.push_back(b); continue; }
        if (!in_frame) continue;
        buf.push_back(b);
        if (b==ETX) break;
      }
      responses[i]=buf;
    }

    auto h = [&](std::string s,int a,int b)->int{
      if (s.size()<b) return 0;
      return std::stoi(s.substr(a,b-a),nullptr,16);
    };

    auto payload = [&](std::string &s)->std::string{
      if (s.size()<3) return "";
      return s.substr(1,s.size()-2);
    };

    for (int i=0;i<7;i++) {
      responses[i]=payload(responses[i]);
    }

    hours->publish_state(h(responses[2],3,11)/4.0/60.0/60.0);
    smoke_chamber->publish_state(h(responses[3],3,7)*0.003223);
    battery_voltage->publish_state(h(responses[4],3,7)*0.018369);
    temp1->publish_state((h(responses[4],7,9)/2.0)-20);
    temp2->publish_state((h(responses[4],9,11)/2.0)-20);
    alarm_count->publish_state(h(responses[3],7,9));
  }
};

}  // namespace gira_rm
}  // namespace esphome
