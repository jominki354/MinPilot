#include "selfdrive/ui/qt/widgets/ssh_keys.h"

#include "selfdrive/common/params.h"
#include "selfdrive/ui/qt/api.h"
#include "selfdrive/ui/qt/widgets/input.h"

SshControl::SshControl() : ButtonControl("SSH 키", "", "경고: 이 설정은 GitHub 설정의 모든 공용 키에 대한 SSH 액세스 권한을 부여합니다. 본인의 GitHub 사용자 이름 이외에는 입력하지 마십시오. comma 직원은 절대 GitHub 사용자 이름을 요청하지 않습니다.") {
  username_label.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  username_label.setStyleSheet("color: #aaaaaa");
  hlayout->insertWidget(1, &username_label);

  QObject::connect(this, &ButtonControl::clicked, [=]() {
    if (text() == "추가") {
      QString username = InputDialog::getText("GitHub 사용자 이름 입력", this);
      if (username.length() > 0) {
        setText("로딩 중");
        setEnabled(false);
        getUserKeys(username);
      }
    } else {
      params.remove("GithubUsername");
      params.remove("GithubSshKeys");
      refresh();
    }
  });

  refresh();
}

void SshControl::refresh() {
  QString param = QString::fromStdString(params.get("GithubSshKeys"));
  if (param.length()) {
    username_label.setText(QString::fromStdString(params.get("GithubUsername")));
    setText("삭제");
  } else {
    username_label.setText("");
    setText("추가");
  }
  setEnabled(true);
}

void SshControl::getUserKeys(const QString &username) {
  HttpRequest *request = new HttpRequest(this, false);
  QObject::connect(request, &HttpRequest::requestDone, [=](const QString &resp, bool success) {
    if (success) {
      if (!resp.isEmpty()) {
        params.put("GithubUsername", username.toStdString());
        params.put("GithubSshKeys", resp.toStdString());
      } else {
        ConfirmationDialog::alert(QString("'%1' 사용자에게 등록된 키가 없습니다").arg(username), this);
      }
    } else {
      if (request->timeout()) {
        ConfirmationDialog::alert("요청 시간 초과", this);
      } else {
        ConfirmationDialog::alert(QString("'%1' 사용자는 GitHub에 존재하지 않습니다").arg(username), this);
      }
    }

    refresh();
    request->deleteLater();
  });

  request->sendRequest("https://github.com/" + username + ".keys");
}
