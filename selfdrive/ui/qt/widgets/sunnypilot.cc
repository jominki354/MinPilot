#include "selfdrive/ui/qt/widgets/sunnypilot.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <QComboBox>
#include <QAbstractItemView>
#include <QScroller>
#include <QListView>
#include <QListWidget>

#include "selfdrive/common/params.h"
#include "selfdrive/ui/qt/api.h"
#include "selfdrive/ui/qt/widgets/input.h"

#include "selfdrive/ui/ui.h"

static QStringList get_list(const char* path)
{
  QStringList stringList;
  QFile textFile(path);
  if(textFile.open(QIODevice::ReadOnly))
  {
      QTextStream textStream(&textFile);
      while (true)
      {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            stringList.append(line);
      }
  }

  return stringList;
}

ForceCarRecognition::ForceCarRecognition(QWidget* parent): QWidget(parent) {

  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setMargin(20);
  main_layout->setSpacing(20);

  QPushButton* back = new QPushButton("뒤로");
  back->setObjectName("backBtn");
  back->setFixedSize(500, 100);
  connect(back, &QPushButton::clicked, [=]() { emit backPress(); });
  main_layout->addWidget(back, 0, Qt::AlignLeft);

  QListWidget* list = new QListWidget(this);
  list->setStyleSheet("QListView {padding: 40px; background-color: #393939; border-radius: 15px; height: 140px;} QListView::item{height: 100px}");
  QScroller::grabGesture(list->viewport(), QScroller::LeftMouseButtonGesture);
  list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  list->addItem("[-선택 안 함-]");

  QStringList items = get_list("/data/params/d/Cars");
  list->addItems(items);
  list->setCurrentRow(0);

  QString set = QString::fromStdString(Params().get("CarModel"));

  int index = 0;
  for(QString item : items) {
    if(set == item) {
        list->setCurrentRow(index + 1);
        break;
    }
    index++;
  }

  QObject::connect(list, QOverload<QListWidgetItem*>::of(&QListWidget::itemClicked),
    [=](QListWidgetItem* item){

    if(list->currentRow() == 0)
        Params().remove("CarModel");
    else
        Params().put("CarModel", list->currentItem()->text().toStdString());

    emit selectedCar();
    });

  main_layout->addWidget(list);
}

// Toyota Enforce Stock Longitudinal
ToyotaEnforceStockLongitudinal::ToyotaEnforceStockLongitudinal() : ParamControl("ToyotaEnforceStockLongitudinal",
                                                                                "토요타: 순정 가감속 제어 사용",
                                                                                "오픈파일럿의 가감속 제어 대신 토요타 순정 ACC의 가감속 제어를 강제로 사용합니다.",
                                                                                "../assets/offroad/icon_road.png") {
}

// Custom Acc Increments Enabled
CustomAccIncrementsEnabled::CustomAccIncrementsEnabled() : ParamControl("CustomAccIncrementsEnabled",
                                                                        "사용자 지정 바로가기 속도 사용",
                                                                        "크루즈 컨트롤 속도 조절 버튼을 눌렀을 때의 속도 변경 단위를 직접 설정합니다.",
                                                                        "../assets/offroad/icon_road.png") {
}

// Custom Acc Increments Short
CustomAccIncrementsShort::CustomAccIncrementsShort() : AbstractControl("짧게 누르기 속도 변경 단위",
                                                                       "크루즈 컨트롤 속도 조절 버튼을 짧게 눌렀을 때 변경될 속도 단위를 설정합니다.",
                                                                       "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("CustomAccIncrementsShort"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 1 ) {
      value = 1;
    }
    QString values = QString::number(value);
    params.put("CustomAccIncrementsShort", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("CustomAccIncrementsShort"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 10 ) {
      value = 10;
    }
    QString values = QString::number(value);
    params.put("CustomAccIncrementsShort", values.toStdString());
    refresh();
  });
  refresh();
}

void CustomAccIncrementsShort::refresh() {
  QString option = QString::fromStdString(params.get("CustomAccIncrementsShort"));
  if (option == "0") {
     params.put("CustomAccIncrementsShort", "1");
     label.setText(QString::fromStdString("1") + "km/h");
  } else {
     label.setText(QString::fromStdString(params.get("CustomAccIncrementsShort")) + "km/h");
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Custom Acc Increments Long
CustomAccIncrementsLong::CustomAccIncrementsLong() : AbstractControl("길게 누르기 속도 변경 단위",
                                                                       "크루즈 컨트롤 속도 조절 버튼을 길게 눌렀을 때 변경될 속도 단위를 설정합니다.",
                                                                       "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("CustomAccIncrementsLong"));
    int value = str.toInt();
    value = value - 5;
    if (value <= 5 ) {
      value = 5;
    }
    QString values = QString::number(value);
    params.put("CustomAccIncrementsLong", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("CustomAccIncrementsLong"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 20 ) {
      value = 20;
    }
    QString values = QString::number(value);
    params.put("CustomAccIncrementsLong", values.toStdString());
    refresh();
  });
  refresh();
}

void CustomAccIncrementsLong::refresh() {
  QString option = QString::fromStdString(params.get("CustomAccIncrementsLong"));
  if (option == "0") {
     params.put("CustomAccIncrementsLong", "5");
     label.setText(QString::fromStdString("5") + "km/h");
  } else {
     label.setText(QString::fromStdString(params.get("CustomAccIncrementsLong")) + "km/h");
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Auto Lane Change Timer (ALCT)
AutoLaneChangeTimer::AutoLaneChangeTimer() : AbstractControl("자동 차선 변경 타이머",
                                                             "방향지시등 사용 시 자동 차선 변경 작동을 지연시킵니다. 타이머가 설정된 경우 핸들을 살짝 돌려주지 않아도 됩니다. (Nudgeless)\n이 기능 사용 시 주의하십시오. 교통 및 도로 상황이 안전할 때만 사용하십시오.",
                                                             "../assets/offroad/icon_road.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 50px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("AutoLaneChangeTimer"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    }
    QString values = QString::number(value);
    params.put("AutoLaneChangeTimer", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("AutoLaneChangeTimer"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 5 ) {
      value = 5;
    }
    QString values = QString::number(value);
    params.put("AutoLaneChangeTimer", values.toStdString());
    refresh();
  });
  refresh();
}

void AutoLaneChangeTimer::refresh() {
  QString option = QString::fromStdString(params.get("AutoLaneChangeTimer"));
  if (option == "0") {
    label.setText(QString::fromStdString("Nudge"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("바로 (Nudgeless)"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("0.5초"));
  } else if (option == "3") {
    label.setText(QString::fromStdString("1초"));
  } else if (option == "4") {
    label.setText(QString::fromStdString("1.5초"));
  } else {
    label.setText(QString::fromStdString("2초"));
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Brightness Control (Global)
BrightnessControl::BrightnessControl() : AbstractControl("화면 밝기 조절 (전체, %)",
                                                         "전체 화면의 밝기를 수동으로 조절합니다.",
                                                         "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("BrightnessControl"));
    int value = str.toInt();
    value = value - 5;
    if (value <= 0 ) {
      value = 0;
    }
    QUIState::ui_state.scene.brightness = value;
    QString values = QString::number(value);
    params.put("BrightnessControl", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("BrightnessControl"));
    int value = str.toInt();
    value = value + 5;
    if (value >= 100 ) {
      value = 100;
    }
    QUIState::ui_state.scene.brightness = value;
    QString values = QString::number(value);
    params.put("BrightnessControl", values.toStdString());
    refresh();
  });
  refresh();
}

void BrightnessControl::refresh() {
  QString option = QString::fromStdString(params.get("BrightnessControl"));
  if (option == "0") {
    label.setText(QString::fromStdString("자동"));
  } else {
    label.setText(QString::fromStdString(params.get("BrightnessControl")));
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Onroad Screen Off (Auto Onroad Screen Timer)
OnroadScreenOff::OnroadScreenOff() : AbstractControl("주행 중 화면 끄기",
                                                     "주행 시작 후 일정 시간이 지나면 화면을 끄거나 어둡게 하여 화면 번인을 방지합니다. 화면을 터치하거나 이벤트가 발생하면 다시 켜집니다.",
                                                     "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("OnroadScreenOff"));
    int value = str.toInt();
    value = value - 1;
    if (value <= -2 ) {
      value = -2;
    }
    QUIState::ui_state.scene.onroadScreenOff = value;
    QString values = QString::number(value);
    params.put("OnroadScreenOff", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("OnroadScreenOff"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 10 ) {
      value = 10;
    }
    QUIState::ui_state.scene.onroadScreenOff = value;
    QString values = QString::number(value);
    params.put("OnroadScreenOff", values.toStdString());
    refresh();
  });
  refresh();
}

void OnroadScreenOff::refresh()
{
  QString option = QString::fromStdString(params.get("OnroadScreenOff"));
  if (option == "-2") {
    label.setText(QString::fromStdString("항상 켜짐"));
  } else if (option == "-1") {
    label.setText(QString::fromStdString("15초"));
  } else if (option == "0") {
    label.setText(QString::fromStdString("30초"));
  } else {
    label.setText(QString::fromStdString(params.get("OnroadScreenOff")) + "분");
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Onroad Screen Off Brightness
OnroadScreenOffBrightness::OnroadScreenOffBrightness() : AbstractControl("화면 끄기 모드 밝기 (%)",
                                                                         "주행 중 화면 끄기 기능이 작동할 때의 화면 밝기를 설정합니다.",
                                                                         "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("OnroadScreenOffBrightness"));
    int value = str.toInt();
    value = value - 10;
    if (value <= 0 ) {
      value = 0;
    }
    QUIState::ui_state.scene.onroadScreenOffBrightness = value;
    QString values = QString::number(value);
    params.put("OnroadScreenOffBrightness", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("OnroadScreenOffBrightness"));
    int value = str.toInt();
    value = value + 10;
    if (value >= 100 ) {
      value = 100;
    }
    QUIState::ui_state.scene.onroadScreenOffBrightness = value;
    QString values = QString::number(value);
    params.put("OnroadScreenOffBrightness", values.toStdString());
    refresh();
  });
  refresh();
}

void OnroadScreenOffBrightness::refresh() {
  QString option = QString::fromStdString(params.get("OnroadScreenOffBrightness"));
  if (option == "0") {
    label.setText(QString::fromStdString("가장 어둡게"));
  } else {
    label.setText(QString::fromStdString(params.get("OnroadScreenOffBrightness")));
  }
  btnminus.setText("-");
  btnplus.setText("+");
}

// Max Time Offroad (Shutdown timer)
MaxTimeOffroad::MaxTimeOffroad() : AbstractControl("주차 후 전원 끄기 타이머",
                                                   "시동을 끈 후(오프로드 상태) 설정된 시간이 지나면 장치 전원을 자동으로 끕니다.",
                                                   "../assets/offroad/icon_metric.png") {

  label.setAlignment(Qt::AlignVCenter|Qt::AlignRight);
  label.setStyleSheet("color: #e0e879");
  hlayout->addWidget(&label);

  btnminus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnplus.setStyleSheet(R"(
    padding: 0;
    border-radius: 50px;
    font-size: 35px;
    font-weight: 500;
    color: #E4E4E4;
    background-color: #393939;
  )");
  btnminus.setFixedSize(150, 100);
  btnplus.setFixedSize(150, 100);
  hlayout->addWidget(&btnminus);
  hlayout->addWidget(&btnplus);

  QObject::connect(&btnminus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("MaxTimeOffroad"));
    int value = str.toInt();
    value = value - 1;
    if (value <= 0 ) {
      value = 0;
    }
    QString values = QString::number(value);
    params.put("MaxTimeOffroad", values.toStdString());
    refresh();
  });

  QObject::connect(&btnplus, &QPushButton::clicked, [=]() {
    auto str = QString::fromStdString(params.get("MaxTimeOffroad"));
    int value = str.toInt();
    value = value + 1;
    if (value >= 12 ) {
      value = 12;
    }
    QString values = QString::number(value);
    params.put("MaxTimeOffroad", values.toStdString());
    refresh();
  });
  refresh();
}

void MaxTimeOffroad::refresh() {
  QString option = QString::fromStdString(params.get("MaxTimeOffroad"));
  if (option == "0") {
    label.setText(QString::fromStdString("항상 켜짐"));
  } else if (option == "1") {
    label.setText(QString::fromStdString("즉시"));
  } else if (option == "2") {
    label.setText(QString::fromStdString("30초"));
  } else if (option == "3") {
    label.setText(QString::fromStdString("1분"));
  } else if (option == "4") {
    label.setText(QString::fromStdString("3분"));
  } else if (option == "5") {
    label.setText(QString::fromStdString("5분"));
  } else if (option == "6") {
    label.setText(QString::fromStdString("10분"));
  } else if (option == "7") {
    label.setText(QString::fromStdString("30분"));
  } else if (option == "8") {
    label.setText(QString::fromStdString("1시간"));
  } else if (option == "9") {
    label.setText(QString::fromStdString("3시간"));
  } else if (option == "10") {
    label.setText(QString::fromStdString("5시간"));
  } else if (option == "11") {
    label.setText(QString::fromStdString("10시간"));
  } else if (option == "12") {
    label.setText(QString::fromStdString("30시간"));
  }
  btnminus.setText("-");
  btnplus.setText("+");
}