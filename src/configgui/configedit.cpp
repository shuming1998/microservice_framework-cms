#include "configedit.h"
#include "clogclient.h"
#include "cconfigclient.h"
#include "ctools.h"
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>
#include <string>
#include <fstream>

static ConfigEdit *curEdit = nullptr;

static void configMessageCb(bool isOk, const char *msg) {
  if (curEdit) {
    curEdit->messageCbSig(isOk, msg);
  }
}

ConfigEdit::ConfigEdit(QWidget *parent) : QDialog(parent) {
  curEdit = this;
  ui.setupUi(this);
  setLayout(ui.formLayout);
  // 关联信号槽
  QObject::connect(this, SIGNAL(messageCbSig(bool, const char *)), this, SLOT(messageCb(bool, const char *)));
  // 设定用于上传配置后反馈信息的回调函数，用于判断界面是否应该刷新
  CConfigClient::get()->uploadConfigResCb_ = configMessageCb;
  // 记录配置的基础信息行数
  configRowCount_ = ui.formLayout->rowCount();
  if (!config_) {
    config_ = new cmsg::CConfig();
  }
}

ConfigEdit::~ConfigEdit() {
  curEdit = nullptr;
  if (config_) {
    delete config_;
    config_ = nullptr;
  }
}

void ConfigEdit::messageCb(bool isOk, const char *msg) {
  if (!isOk) {
    QMessageBox::information(this, "", msg);
    return;
  }
  accept();
}

void ConfigEdit::save() {
  if (!message_) {
    LOG_DEBUG("save failed: message_ is nullptr!");
    QMessageBox::information(this, "", "proto file not set!");
    return;
  }

  if (ui.serviceNameLineEdit->text().isEmpty()
      || ui.serviceIpLineEdit->text().isEmpty()
      || ui.protoTextEdit->toPlainText().isEmpty()) {
    QMessageBox::information(this, "", "make sure serviceName/serviceIp/protoType has valid value!");
    return;
  }
  
  // 遍历界面 区分 基础信息(存于 CConfig) 和 配置信息
  // 配置信息，将界面输入存储到 message 中
  // 获取类型描述
  auto desc = message_->GetDescriptor();
  // message 反射
  auto ref = message_->GetReflection();
  // 遍历输入
  for (int i = configRowCount_; i < ui.formLayout->rowCount(); ++i) {
    // 从 label 的 text 中找到 key 
    auto labelItem = ui.formLayout->itemAt(i, QFormLayout::LabelRole);
    if (!labelItem) {
      continue;
    }
    // 运行时转换，失败返回 nullptr
    auto label = dynamic_cast<QLabel *>(labelItem->widget());
    if (!label) {
      continue;
    }
    // 从 label 中获取到了 key
    auto fieldName = label->text().toStdString();

    // 获取 value，输入控件中的值，枚举、整形、浮点、字符串
    //  获取控件
    auto fieldItem = ui.formLayout->itemAt(i, QFormLayout::FieldRole);
    if (!fieldItem) {
      continue;
    }
    auto fieldEdit = fieldItem->widget();
    //  获取字段描述符(类型)
    auto fieldDesc = desc->FindFieldByName(fieldName);
    if (!fieldDesc) {
      continue;
    }
    auto type = fieldDesc->type();
    // 获取控件的值，设置到 message
    QSpinBox *intBox = nullptr;           // 整形
    QDoubleSpinBox *doubleBox = nullptr;  // 浮点
    QLineEdit *strEdit = nullptr;         // 字符串和 byte
    QComboBox *comboBox = nullptr;        // 布尔和枚举
    // 获取控件的值，设置到 message (反射)
    switch (type) {
      case google::protobuf::FieldDescriptor::TYPE_INT64:
        intBox = dynamic_cast<QSpinBox *>(fieldEdit);
        if (!intBox) continue;
        ref->SetInt64(message_, fieldDesc, intBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_INT32:
        intBox = dynamic_cast<QSpinBox *>(fieldEdit);
        if (!intBox) continue;
        ref->SetInt32(message_, fieldDesc, intBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
        doubleBox = dynamic_cast<QDoubleSpinBox *>(fieldEdit);
        if (!doubleBox) continue;
        ref->SetDouble(message_, fieldDesc, doubleBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_FLOAT:
        doubleBox = dynamic_cast<QDoubleSpinBox *>(fieldEdit);
        if (!doubleBox) continue;
        ref->SetFloat(message_, fieldDesc, doubleBox->value());
        break;
      case google::protobuf::FieldDescriptor::TYPE_BYTES:
      case google::protobuf::FieldDescriptor::TYPE_STRING:
        strEdit = dynamic_cast<QLineEdit *>(fieldEdit);
        if (!strEdit) continue;
        ref->SetString(message_, fieldDesc, strEdit->text().toStdString());
        break;
      case google::protobuf::FieldDescriptor::TYPE_BOOL:
        comboBox = dynamic_cast<QComboBox *>(fieldEdit);
        if (!comboBox) continue;
        ref->SetBool(message_, fieldDesc, comboBox->currentData().toBool());
        break;
      case google::protobuf::FieldDescriptor::TYPE_ENUM:
        comboBox = dynamic_cast<QComboBox *>(fieldEdit);
        if (!comboBox) continue;
        ref->SetEnumValue(message_, fieldDesc, comboBox->currentData().toInt());
        break;
      default:
        break;
    }
  }

  // 基础信息
  cmsg::CConfig config;
  config.set_servicename(ui.serviceNameLineEdit->text().toStdString());
  config.set_serviceip(ui.serviceIpLineEdit->text().toStdString());
  config.set_serviceport(ui.servicePortSpinBox->value());
  config.set_proto(ui.protoTextEdit->toPlainText().toStdString());
  
  // 序列化 message
  std::string msgPb = message_->SerializeAsString();
  config.set_privatepb(msgPb);

  LOG_DEBUG(message_->DebugString());
  LOG_DEBUG(config.DebugString());
  // 上传配置到配置中心
  CConfigClient::get()->uploadConfig(&config);
}

void ConfigEdit::loadProto() {
  LOG_DEBUG("load Proto");

  // 用户输入类型名称，如果没有名称，则使用 proto 文件中的第一个类型
  QString className = ui.typeLineEdit->text();
  std::string classNameStr = "";
  if (!className.isEmpty()) {
    classNameStr = className.toStdString();
  }

  // 用户选择 proto 文件
  QString filename = QFileDialog::getOpenFileName(this,
    QString::fromLocal8Bit("请选择 proto 文件"), "", "*.proto");
  if (filename.isEmpty()) {
    return;
  }

  //LOG_DEBUG(filename.toStdString().c_str());

  // 获取反射的 message 对象
  std::string protoCode = "";
  message_ = CConfigClient::get()->loadProto(filename.toStdString(), classNameStr, protoCode);
  if (!message_) {
    LOG_DEBUG("CConfigClient::get()->loadProto failed!")
    return;
  }
  config_->set_proto(protoCode);
  initGui();
}

bool ConfigEdit::loadConfig(const char *ip, int port) {
  // 发送消息获取配置项 CConfig，存储到成员 config_ 中
  CConfigClient::get()->downloadConfig(ip, port);
  if (!config_)
    config_ = new cmsg::CConfig();
  bool isGet = false;

  // 超时等待一秒
  for (int i = 0; i < 100; ++i) {
    if (CConfigClient::get()->getConfig(ip, port, config_)) {
      isGet = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (!isGet) {
    LOG_DEBUG("获取需要修改的配置数据失败!");
    return false;
  }
  LOG_DEBUG("获取需要修改的配置数据成功!");
  if (config_->proto().empty()) {
    LOG_DEBUG("配置的 proto 为空!");
    return false;
  }
  
  // 将配置项写入 proto 文件
  std::string protoFilePath = "tmp.proto";
  std::ofstream ofs(protoFilePath);
  ofs << config_->proto();
  ofs.close();

  // 加载 proto，生成 message
  std::string outProto = "";
  // 取 proto 文件中的第一个类型
  message_ = CConfigClient::get()->loadProto(protoFilePath, "", outProto);
  if (!message_) {
    LOG_DEBUG("加载 proto 文件失败!");
    return false;
  }
  //LOG_DEBUG(config_->DebugString());
  //LOG_DEBUG(message_->GetDescriptor()->DebugString());

  // message 反序列化，生成 proto 文件并加载
  if (!message_->ParseFromString(config_->privatepb())) {
    LOG_DEBUG("反序列化 message 失败!");
    return false;
  }
  LOG_DEBUG(message_->DebugString().c_str());

  initGui();
  return true;
}

void ConfigEdit::initGui() {
  // 先清理之前记录的配置信息项
  while (ui.formLayout->rowCount() != configRowCount_) {
    ui.formLayout->removeRow(configRowCount_);
  }

  // 服务配置的基础信息
  if (config_) {
    ui.serviceIpLineEdit->setText(config_->serviceip().c_str());
    ui.serviceNameLineEdit->setText(config_->servicename().c_str());
    ui.servicePortSpinBox->setValue(config_->serviceport());
    ui.protoTextEdit->setText(config_->proto().c_str());
  }

  if (!message_) {
    return;
  }

  ui.typeLineEdit->setText(message_->GetTypeName().c_str());

  // 通过反射生成 message 界面，并设定值
  // 获取类型描述
  auto desc = message_->GetDescriptor();
  // 通过反射设定界面的值
  auto ref = message_->GetReflection();
  // 遍历字段
  int fieldCount = desc->field_count();
  for (int i = 0; i < fieldCount; ++i) {
    // 单个字段描述
    auto field = desc->field(i);
    auto type = field->type();
    // 支持:数字、字符串、枚举
    QSpinBox *intBox = nullptr;           // 整形
    QDoubleSpinBox *doubleBox = nullptr;  // 浮点
    QLineEdit *strEdit = nullptr;         // 字符串和 byte
    QComboBox *comboBox = nullptr;        // 布尔和枚举
    switch (type) {
      // 支持整形数字
    case google::protobuf::FieldDescriptor::TYPE_INT64:
      intBox = new QSpinBox();
      intBox->setMaximum(INT64_MAX);
      intBox->setValue(ref->GetInt64(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), intBox);
      break;
    case google::protobuf::FieldDescriptor::TYPE_INT32:
      intBox = new QSpinBox();
      intBox->setMaximum(INT32_MAX);
      intBox->setValue(ref->GetInt32(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), intBox);
      break;
      // 支持浮点数
    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
      doubleBox = new QDoubleSpinBox();
      doubleBox->setMaximum(DBL_MAX);
      doubleBox->setValue(ref->GetDouble(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), doubleBox);
      break;
    case google::protobuf::FieldDescriptor::TYPE_FLOAT:
      doubleBox = new QDoubleSpinBox();
      doubleBox->setMaximum(FLT_MAX);
      doubleBox->setValue(ref->GetFloat(*message_, field));
      ui.formLayout->addRow(field->name().c_str(), doubleBox);
      break;
      // 支持字符串和 byte
    case google::protobuf::FieldDescriptor::TYPE_BYTES:
    case google::protobuf::FieldDescriptor::TYPE_STRING:
      strEdit = new QLineEdit();
      strEdit->setText(ref->GetString(*message_, field).c_str());
      ui.formLayout->addRow(field->name().c_str(), strEdit);
      break;
      // 支持 bool 类型
    case google::protobuf::FieldDescriptor::TYPE_BOOL:
      comboBox = new QComboBox();
      comboBox->addItem("true", true);
      comboBox->addItem("false", false);
      if (ref->GetBool(*message_, field)) {
        comboBox->setCurrentIndex(0);
      } else {
        comboBox->setCurrentIndex(1);
      }
      ui.formLayout->addRow(field->name().c_str(), comboBox);
      break;
      // 支持枚举类型
    case google::protobuf::FieldDescriptor::TYPE_ENUM:
      comboBox = new QComboBox();
      for (int j = 0; j < field->enum_type()->value_count(); ++j) {
        // 获取枚举名
        std::string enumName = field->enum_type()->value(j)->name();
        // 获取该枚举名对应的值
        int enumVal = field->enum_type()->value(j)->number();
        // 插入该枚举选项
        comboBox->addItem(enumName.c_str(), enumVal);
      }
      ui.formLayout->addRow(field->name().c_str(), comboBox);
      // 根据对应的 index 获取值
      comboBox->setCurrentIndex(comboBox->findData(ref->GetEnumValue(*message_, field)));
      break;
    default:
      break;
    }
  }
}

