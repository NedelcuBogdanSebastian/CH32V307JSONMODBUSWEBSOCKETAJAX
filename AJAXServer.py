from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

global k
k = 0

@app.route('/echo', methods=['GET'])
def echo():
    global k
    data_received = request.args.get('data', 'No data provided')

    k += 1
    print(f"Data received: {data_received}")  # This should print to your terminal

    response = {
        'message': 'You sent:',
        'data': data_received + str(k)
    }
    return jsonify(response)

if __name__ == '__main__':
    app.run(host='localhost', port=80, debug=True)
