#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <cmath>
#include <string>

#include <QDebug>

#ifndef QCOM
#include "selfdrive/ui/qt/offroad/networking.h"
#endif

#ifdef ENABLE_MAPS
#include "selfdrive/ui/qt/maps/map_settings.h"
#endif

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/sunnypilot.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"

TogglesPanel::TogglesPanel(SettingsWindow *parent) : ListWidget(parent) {
  // param, title, desc, icon
  std::vector<std::tuple<QString, QString, QString, QString>> toggles{
    {
      "OpenpilotEnabledToggle",
      "오픈파일럿 사용",
      "오픈파일럿 시스템을 사용하여 적응형 크루즈 컨트롤 및 차선 유지 보조 기능을 사용합니다. 이 기능을 사용할 때는 항상 주의가 필요합니다. 설정을 변경하면 시동을 껐다 켤 때 적용됩니다.",
      "../assets/offroad/icon_openpilot.png",
    },
    {
      "IsLdwEnabled",
      "차선 이탈 경보",
      "차량이 깜빡이 없이 차선을 넘으려 할 때 50km/h(31mph) 이상에서 경고를 보냅니다.",
      "../assets/offroad/icon_warning.png",
    },
    {
      "IsRHD",
      "우핸들 차량 모드",
      "오픈파일럿이 좌측 통행 규칙을 따르고 우측 운전석을 모니터링하도록 허용합니다.",
      "../assets/offroad/icon_openpilot_mirrored.png",
    },
    {
      "IsMetric",
      "미터법 사용 (km/h)",
      "속도를 mph 대신 km/h로 표시합니다.",
      "../assets/offroad/icon_metric.png",
    },
    {
      "CommunityFeaturesToggle",
      "커뮤니티 기능 (실험적 기능)",
      "comma.ai에서 공식적으로 유지 관리하거나 지원하지 않으며 표준 안전 모델을 충족하는지 확인되지 않은 오픈 소스 커뮤니티의 기능(예: 커뮤니티 지원 하드웨어)을 사용합니다. 이 기능을 사용할 때는 각별히 주의하십시오.",
      "../assets/offroad/icon_shell.png",
    },
    {
      "RecordFront",
      "운전자 영상 녹화 및 공유",
      "운전자 카메라 데이터를 업로드하여 운전자 모니터링 알고리즘 개선을 돕습니다.",
      "../assets/offroad/icon_monitoring.png",
    },
    {
      "EndToEndToggle",
      "\U0001f96c 차선 무시 주행 (Laneless 모드) \U0001f96c",
      "이 모드에서는 오픈파일럿이 차선 정보를 무시하고 사람이 운전하는 것처럼 주행하려고 합니다.",
      "../assets/offroad/icon_road.png",
    },
#ifdef ENABLE_MAPS
    {
      "NavSettingTime24h",
      "Show ETA in 24h format",
      "Use 24h format instead of am/pm",
      "../assets/offroad/icon_metric.png",
    },
#endif

  };

  Params params;

  if (params.getBool("DisableRadar_Allow")) {
    toggles.push_back({
      "DisableRadar",
      "openpilot Longitudinal Control",
      "openpilot will disable the car's radar and will take over control of gas and brakes. Warning: this disables AEB!",
      "../assets/offroad/icon_speed_limit.png",
    });
  }

  for (auto &[param, title, desc, icon] : toggles) {
    auto toggle = new ParamControl(param, title, desc, icon, this);
    bool locked = params.getBool((param + "Lock").toStdString());
    toggle->setEnabled(!locked);
    if (!locked) {
      connect(parent, &SettingsWindow::offroadTransition, toggle, &ParamControl::setEnabled);
    }
    addItem(toggle);
  }
}

DevicePanel::DevicePanel(SettingsWindow *parent) : ListWidget(parent) {
  setSpacing(50);
  addItem(new LabelControl("동글 ID", getDongleId().value_or("N/A")));
  addItem(new LabelControl("시리얼", params.get("HardwareSerial").c_str()));

  // offroad-only buttons

  auto dcamBtn = new ButtonControl("운전자 카메라", "미리보기",
                                   "차량 내부 카메라의 각도를 확인합니다.");
  connect(dcamBtn, &ButtonControl::clicked, [=]() { emit showDriverView(); });
  addItem(dcamBtn);

  auto resetCalibBtn = new ButtonControl("캘리브레이션 초기화", "초기화", " ");
  connect(resetCalibBtn, &ButtonControl::showDescription, this, &DevicePanel::updateCalibDescription);
  connect(resetCalibBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm("캘리브레이션을 초기화하시겠습니까?", this)) {
      params.remove("CalibrationParams");
    }
  });
  addItem(resetCalibBtn);

  if (!params.getBool("Passive")) {
    auto retrainingBtn = new ButtonControl("트레이닝 가이드 다시보기", "보기", "오픈파일럿의 규칙, 기능 및 제한 사항을 다시 확인합니다.");
    connect(retrainingBtn, &ButtonControl::clicked, [=]() {
      if (ConfirmationDialog::confirm("트레이닝 가이드를 다시 보시겠습니까?", this)) {
        emit reviewTrainingGuide();
      }
    });
    addItem(retrainingBtn);
  }

  if (Hardware::TICI()) {
    auto regulatoryBtn = new ButtonControl("규제 정보", "보기", "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file("../assets/offroad/fcc.html");
      RichTextDialog::alert(QString::fromStdString(txt), this);
    });
    addItem(regulatoryBtn);
  }

  QObject::connect(parent, &SettingsWindow::offroadTransition, [=](bool offroad) {
    for (auto btn : findChildren<ButtonControl *>()) {
      btn->setEnabled(offroad);
    }
  });

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("재부팅");
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, this, &DevicePanel::reboot);

  QPushButton *poweroff_btn = new QPushButton("전원 끄기");
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, this, &DevicePanel::poweroff);

  setStyleSheet(R"(
    #reboot_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { height: 120px; border-radius: 15px; background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  addItem(power_layout);
}

void DevicePanel::updateCalibDescription() {
  QString desc =
      "오픈파일럿은 장치가 좌우 4°, 상하 5° 이내로 장착되어야 합니다. 오픈파일럿은 지속적으로 캘리브레이션을 수행하므로 재설정이 거의 필요하지 않습니다.";
  std::string calib_bytes = Params().get("CalibrationParams");
  if (!calib_bytes.empty()) {
    try {
      AlignedBuffer aligned_buf;
      capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
      auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
      if (calib.getCalStatus() != 0) {
        double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
        double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
        desc += QString(" 장치가 %2 %1°, %4 %3° 방향을 향하고 있습니다.")
                    .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "위로" : "아래로",
                         QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "우측으로" : "좌측으로");
      }
    } catch (kj::Exception) {
      qInfo() << "invalid CalibrationParams";
    }
  }
  qobject_cast<ButtonControl *>(sender())->setDescription(desc);
}

void DevicePanel::reboot() {
  if (QUIState::ui_state.status == UIStatus::STATUS_DISENGAGED) {
    if (ConfirmationDialog::confirm("재부팅하시겠습니까?", this)) {
      // Check engaged again in case it changed while the dialog was open
      if (QUIState::ui_state.status == UIStatus::STATUS_DISENGAGED) {
        Params().putBool("DoReboot", true);
      }
    }
  } else {
    ConfirmationDialog::alert("주행 중에는 재부팅할 수 없습니다", this);
  }
}

void DevicePanel::poweroff() {
  if (QUIState::ui_state.status == UIStatus::STATUS_DISENGAGED) {
    if (ConfirmationDialog::confirm("전원을 끄시겠습니까?", this)) {
      // Check engaged again in case it changed while the dialog was open
      if (QUIState::ui_state.status == UIStatus::STATUS_DISENGAGED) {
        Params().putBool("DoShutdown", true);
      }
    }
  } else {
    ConfirmationDialog::alert("주행 중에는 전원을 끌 수 없습니다", this);
  }
}

SoftwarePanel::SoftwarePanel(QWidget* parent) : ListWidget(parent) {
  gitBranchLbl = new LabelControl("Git 브랜치");
  gitCommitLbl = new LabelControl("Git 커밋");
  osVersionLbl = new LabelControl("OS 버전");
  versionLbl = new LabelControl("버전", "", QString::fromStdString(params.get("ReleaseNotes")).trimmed());
  lastUpdateLbl = new LabelControl("마지막 업데이트 확인", "", "마지막으로 업데이트를 확인한 시간입니다. 업데이트는 시동이 꺼져 있을 때만 실행됩니다.");
  updateBtn = new ButtonControl("업데이트 확인", "");
  connect(updateBtn, &ButtonControl::clicked, [=]() {
    if (params.getBool("IsOffroad")) {
      fs_watch->addPath(QString::fromStdString(params.getParamPath("LastUpdateTime")));
      fs_watch->addPath(QString::fromStdString(params.getParamPath("UpdateFailedCount")));
      updateBtn->setText("확인 중");
      updateBtn->setEnabled(false);
    }
    std::system("pkill -1 -f selfdrive.updated");
  });


  auto uninstallBtn = new ButtonControl(getBrand() + " 제거", "제거");
  connect(uninstallBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm("정말 제거하시겠습니까?", this)) {
      params.putBool("DoUninstall", true);
    }
  });
  connect(parent, SIGNAL(offroadTransition(bool)), uninstallBtn, SLOT(setEnabled(bool)));

  QWidget *widgets[] = {versionLbl, lastUpdateLbl, updateBtn, gitBranchLbl, gitCommitLbl, osVersionLbl, uninstallBtn};
  for (QWidget* w : widgets) {
    addItem(w);
  }

  fs_watch = new QFileSystemWatcher(this);
  QObject::connect(fs_watch, &QFileSystemWatcher::fileChanged, [=](const QString path) {
    if (path.contains("UpdateFailedCount") && std::atoi(params.get("UpdateFailedCount").c_str()) > 0) {
      lastUpdateLbl->setText("업데이트 확인 실패");
      updateBtn->setText("확인");
      updateBtn->setEnabled(true);
    } else if (path.contains("LastUpdateTime")) {
      updateLabels();
    }
  });
}

void SoftwarePanel::showEvent(QShowEvent *event) {
  updateLabels();
}

void SoftwarePanel::updateLabels() {
  QString lastUpdate = "";
  auto tm = params.get("LastUpdateTime");
  if (!tm.empty()) {
    lastUpdate = timeAgo(QDateTime::fromString(QString::fromStdString(tm + "Z"), Qt::ISODate));
  }

  versionLbl->setText(getBrandVersion());
  lastUpdateLbl->setText(lastUpdate);
  updateBtn->setText("확인");
  updateBtn->setEnabled(true);
  gitBranchLbl->setText(QString::fromStdString(params.get("GitBranch")));
  gitCommitLbl->setText(QString::fromStdString(params.get("GitCommit")).left(10));
  osVersionLbl->setText(QString::fromStdString(Hardware::get_os_version()).trimmed());
}

QWidget * network_panel(QWidget * parent) {
#ifdef QCOM
  QWidget *w = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout(w);
  layout->setContentsMargins(50, 0, 50, 0);

  ListWidget *list = new ListWidget();
  list->setSpacing(30);
  // wifi + tethering buttons
  auto wifiBtn = new ButtonControl("Wi-Fi Settings", "OPEN");
  QObject::connect(wifiBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_wifi(); });
  list->addItem(wifiBtn);

  auto tetheringBtn = new ButtonControl("Tethering Settings", "OPEN");
  QObject::connect(tetheringBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_tethering(); });
  list->addItem(tetheringBtn);

  // SSH key management
  list->addItem(new SshToggle());
  list->addItem(new SshControl());

  layout->addWidget(list);
  layout->addStretch(1);
#else
  Networking *w = new Networking(parent);
#endif
  return w;
}

void SettingsWindow::showEvent(QShowEvent *event) {
  panel_widget->setCurrentIndex(0);
  nav_btns->buttons()[0]->setChecked(true);
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("×");
  close_btn->setStyleSheet(R"(
    QPushButton {
      font-size: 140px;
      padding-bottom: 20px;
      font-weight: bold;
      border 1px grey solid;
      border-radius: 100px;
      background-color: #292929;
      font-weight: 400;
    }
    QPushButton:pressed {
      background-color: #3B3B3B;
    }
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, &QPushButton::clicked, this, &SettingsWindow::closeSettings);

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  QList<QPair<QString, QWidget *>> panels = {
    {"장치", device},
    {"네트워크", network_panel(this)},
    {"토글", new TogglesPanel(this)},
    {"소프트웨어", new SoftwarePanel(this)},
    {"써니파일럿", new SunnypilotPanel(this)},
  };

#ifdef ENABLE_MAPS
  auto map_panel = new MapPanel(this);
  panels.push_back({"내비게이션", map_panel});
  QObject::connect(map_panel, &MapPanel::closeSettings, this, &SettingsWindow::closeSettings);
#endif

  const int padding = panels.size() > 3 ? 15 : 35;

  nav_btns = new QButtonGroup(this);
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(QString(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 65px;
        font-weight: 500;
        padding-top: %1px;
        padding-bottom: %1px;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )").arg(padding));

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    const int lr_margin = name != "네트워크" ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
  )");
}

void SettingsWindow::hideEvent(QHideEvent *event) {
#ifdef QCOM
  HardwareEon::close_activities();
#endif
}

SunnypilotPanel::SunnypilotPanel(QWidget* parent) : QWidget(parent) {
  main_layout = new QStackedLayout(this);
  home = new QWidget(this);
  QVBoxLayout* fcr_layout = new QVBoxLayout(home);
  fcr_layout->setContentsMargins(0, 20, 0, 20);

  QString set = QString::fromStdString(Params().get("CarModel"));

  QPushButton* setCarBtn = new QPushButton(set.length() ? set : "차량 선택");
  setCarBtn->setObjectName("setCarBtn");
  setCarBtn->setStyleSheet("margin-right: 30px;");
  connect(setCarBtn, &QPushButton::clicked, [=]() { main_layout->setCurrentWidget(setCar); });
  fcr_layout->addSpacing(10);
  fcr_layout->addWidget(setCarBtn, 0, Qt::AlignRight);
  fcr_layout->addSpacing(10);

  home_widget = new QWidget(this);
  QVBoxLayout* toggle_layout = new QVBoxLayout(home_widget);
  home_widget->setObjectName("homeWidget");

  ScrollView *scroller = new ScrollView(home_widget, this);
  scroller->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  fcr_layout->addWidget(scroller, 1);

  main_layout->addWidget(home);

  setCar = new ForceCarRecognition(this);
  connect(setCar, &ForceCarRecognition::backPress, [=]() { main_layout->setCurrentWidget(home); });
  connect(setCar, &ForceCarRecognition::selectedCar, [=]() {
    QString set = QString::fromStdString(Params().get("CarModel"));
    setCarBtn->setText(set.length() ? set : "차량 선택");
    main_layout->setCurrentWidget(home);
  });
  main_layout->addWidget(setCar);

  QPalette pal = palette();
  pal.setColor(QPalette::Background, QColor(0x29, 0x29, 0x29));
  setAutoFillBackground(true);
  setPalette(pal);

  setStyleSheet(R"(
    #backBtn, #setCarBtn {
      font-size: 50px;
      margin: 0px;
      padding: 20px;
      border-width: 0;
      border-radius: 30px;
      color: #dddddd;
      background-color: #444444;
    }
  )");

  QList<ParamControl*> toggles;

  toggles.append(new ParamControl("QuietDrive",
                                  "조용한 주행",
                                  "경고음을 최소화하고 화면 알림을 우선합니다.",
                                  "../assets/offroad/icon_mute.png",
                                  this));

  toggles.append(new ParamControl("PrebuiltOn",
                                  "빠른 부팅",
                                  "부팅 속도를 높입니다. UI 변경 시에는 꺼주세요.",
                                  "../assets/offroad/icon_shell.png",
                                  this));

  toggles.append(new ParamControl("DisableOnroadUploads",
                                  "데이터 절약 모드 (주행 중 업로드 차단)",
                                  "주행 중 데이터 업로드를 완전히 비활성화합니다. 핫스팟 데이터 사용량을 줄일 수 있습니다. 지도 기반 기능(속도 제한, 턴 제어)을 사용하려면 끄십시오.",
                                  "../assets/offroad/icon_network.png",
                                  this));

  toggles.append(new ParamControl("ProcessNotRunningOff",
                                 "시스템 오류 메시지 무시 (해결책 아님)",
                                 "오픈파일럿 사용을 방해하는 '시스템 오류' 알림을 표시하지 않습니다. 이 오류가 자주 발생할 떄만 켜십시오.",
                                 "../assets/offroad/icon_shell.png",
                                 this));

  toggles.append(new ParamControl("NoOffroadFix",
                                 "시동 끄기 강제 (배터리 방전 방지)",
                                 "시동을 끈 후 오픈파일럿이 강제로 오프로드 상태로 전환되고 꺼지도록 합니다. 콤마 파워 없이 실행되는 비공식 기기에서 전원이 안 꺼질 때 사용하세요.",
                                 "../assets/offroad/icon_shell.png",
                                 this));

  toggles.append(new ParamControl("ACCMADSCombo",
                                  "크루즈 버튼으로 상시 조향 켜기",
                                  "핸들의 크루즈 버튼(SET/RES)으로 상시 조향(MADS)을 활성화합니다.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("DisableMADS",
                                  "상시 조향(MADS) 끄기",
                                  "이 기능을 켜면 순정 오픈파일럿처럼 동작합니다.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("HandsOnWheelMonitoring",
                                  "핸들 잡음 모니터링 (경고 켜기)",
                                  "운전자가 핸들을 잡고 있는지 모니터링하고 경고합니다.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("TurnVisionControl",
                                  "커브 감속 (카메라 시야 기반)",
                                  "비전 경로 예측을 사용하여 커브길 진입 시 적절한 속도로 감속합니다.",
                                  "../assets/offroad/icon_road.png",
                                  this));

  toggles.append(new ParamControl("ShowDebugUI",
                                  "개발자용 디버그 정보 표시",
                                  "디버깅에 도움이 되는 UI 요소를 화면에 표시합니다.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

  toggles.append(new ParamControl("SpeedLimitControl",
                                  "제한 속도 자동 맞춤",
                                  "지도 데이터와 차량 인터페이스의 속도 제한 정보를 사용하여 크루즈 속도를 도로 제한 속도에 맞춥니다.",
                                  "../assets/offroad/icon_speed_limit.png",
                                  this));

  toggles.append(new ParamControl("SpeedLimitPercOffset",
                                  "제한 속도 여유분 설정 (+@)",
                                  "설정 속도를 실제 제한 속도보다 약간 높게 설정하여 자연스러운 흐름을 유도합니다.",
                                  "../assets/offroad/icon_speed_limit.png",
                                  this));

  toggles.append(new ParamControl("TurnSpeedControl",
                                  "커브 감속 (지도 데이터 기반)",
                                  "지도 데이터의 곡률 정보를 사용하여 커브길 진입 속도를 조절합니다.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("EnableDebugSnapshot",
                                  "화면 터치로 로그 저장 (디버그용)",
                                  "화면 중앙을 터치하면 현재 상태의 스냅샷 파일을 저장합니다.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

  for (ParamControl *toggle : toggles) {
    if (main_layout->count() != 0) {
      toggle_layout->addWidget(horizontal_line());
    }
    toggle_layout->addWidget(toggle);
  }

  toggle_layout->addWidget(horizontal_line());
  toggle_layout->addWidget(new AutoLaneChangeTimer());
  toggle_layout->addWidget(horizontal_line());
  toggle_layout->addWidget(new BrightnessControl());
  toggle_layout->addWidget(horizontal_line());
  toggle_layout->addWidget(new OnroadScreenOff());
  toggle_layout->addWidget(horizontal_line());
  toggle_layout->addWidget(new OnroadScreenOffBrightness());
  toggle_layout->addWidget(horizontal_line());
  toggle_layout->addWidget(new MaxTimeOffroad());
}