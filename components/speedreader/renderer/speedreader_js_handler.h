// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_COMPONENTS_SPEEDREADER_RENDERER_SPEEDREADER_JS_HANDLER_H_
#define BRAVE_COMPONENTS_SPEEDREADER_RENDERER_SPEEDREADER_JS_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "content/public/renderer/render_frame.h"
#include "gin/wrappable.h"

namespace speedreader {

class SpeedreaderJSHandler : public gin::Wrappable<SpeedreaderJSHandler> {
 public:
  static gin::WrapperInfo kWrapperInfo;

  explicit SpeedreaderJSHandler(content::RenderFrame* render_frame);
  SpeedreaderJSHandler(const SpeedreaderJSHandler&) = delete;
  SpeedreaderJSHandler& operator=(const SpeedreaderJSHandler&) = delete;
  ~SpeedreaderJSHandler() override;

  void AddJavaScriptObjectToFrame(v8::Local<v8::Context> context);
  void ResetRemote(content::RenderFrame* render_frame);

 private:
  // gin::WrappableBase
  gin::ObjectTemplateBuilder GetObjectTemplateBuilder(
      v8::Isolate* isolate) override;

  // A function to be called from JS
  void ShowOriginalPage(v8::Isolate* isolate);

  raw_ptr<content::RenderFrame> render_frame_ = nullptr;
};

}  // namespace speedreader

#endif  // BRAVE_COMPONENTS_SPEEDREADER_RENDERER_SPEEDREADER_JS_HANDLER_H_
