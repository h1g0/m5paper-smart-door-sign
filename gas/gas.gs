function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  var rows = sheet.getRange("A2:C3").getValues();

  var status = {
    main: { text: "", size: 8 },
    sub: { text: "", size: 4 }
  };

  for (var i = 0; i < rows.length; i++) {
    var key = String(rows[i][0] || "").toLowerCase().trim();
    var text = String(rows[i][1] || "");
    var size = parseInt(rows[i][2], 10);
    if (isNaN(size) || size <= 0) {
      size = key === "sub" ? 4 : 8;
    }

    if (key === "main" || key === "sub") {
      status[key] = {
        text: text,
        size: size
      };
    }
  }
  
  var output = {
    main: status.main,
    sub: status.sub,
    updated: new Date()
  };
  
  return ContentService.createTextOutput(JSON.stringify(output))
    .setMimeType(ContentService.MimeType.JSON);
}
