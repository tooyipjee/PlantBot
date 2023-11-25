import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: HomePage(),
    );
  }
}

class HomePage extends StatelessWidget {
  Future<void> sendNotification() async {
    var url = "http://your-esp32-ip/update";
    var response = await http.get(Uri.parse(url));
    if (response.statusCode == 200) {
      print('Notification sent');
    } else {
      print('Failed to send notification');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Plant Moisture App'),
      ),
      body: Center(
        child: ElevatedButton(
          onPressed: () {
            sendNotification();
          },
          child: Text('Notify Device'),
        ),
      ),
    );
  }
}
