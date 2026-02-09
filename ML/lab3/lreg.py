import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.metrics import mean_squared_error
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler


class LinearRegression:
    def __init__(self, dim, lr=0.1):
        assert isinstance
        self.lr = lr
        self.w = np.zeros((dim))  # weight vector
        self.grads = {"dw": np.zeros((dim)) + 5}  # gradient

    def forward(self, x):
        y = self.w.T @ x
        return y

    def backward(self, x, y_hat, y):
        assert y_hat.shape == y.shape
        self.grads["dw"] = (1 / x.shape[1]) * ((y_hat - y) @ x.T).T
        assert self.grads["dw"].shape == self.w.shape

    def optimize(self):
        self.w = self.w - self.lr * self.grads["dw"]


def mse(y, y_hat):
    return ((y - y_hat) ** 2).mean()


def mae(y, y_hat):
    return np.abs(y - y_hat).mean()


def r2_score(y, y_hat):
    ss_res = ((y - y_hat) ** 2).sum()
    ss_tot = ((y - y.mean()) ** 2).sum()
    return 1 - ss_res / ss_tot


df = pd.read_csv("ML Assignment 3.docx-EmbeddedFile.xlsm - weatherHistory.csv")
df.columns = df.columns.str.lower()
df.rename(
    columns={
        "wind speed (km/h)": "wind_speed",
        "pressure (millibars)": "pressure",
        "temperature (c)": "temperature",
    },
    inplace=True,
)

df["date"] = pd.to_datetime(df["formatted_date"], utc=True)
df["year"] = df["date"].dt.year
df["month"] = df["date"].dt.month

# Month has to be cyclic. Cannot be linear.
# This is because distance from Dec to Jan is 1
# If taken linear, it would be 11 (= 12 - 1)
# Thus, linear regression needs to wrap around
df["month_sin"] = np.sin(2 * np.pi * df["month"] / 12)
df["month_cos"] = np.cos(2 * np.pi * df["month"] / 12)

df = df.drop(columns=["formatted_date", "summary", "precip_type", "daily summary"])
df.fillna(df.mean(numeric_only=True), inplace=True)

grouped_df = df.groupby(["year", "month"]).mean().reset_index()

print(grouped_df.head())

x = grouped_df[["year", "month_sin", "month_cos"]]
y = grouped_df[["temperature"]]

x_train, x_test, y_train, y_test = train_test_split(
    x, y, test_size=0.33, random_state=1
)

input_scaler = StandardScaler()
output_scaler = StandardScaler()

x_train = input_scaler.fit_transform(x_train).T
x_test = input_scaler.transform(x_test).T

y_train = output_scaler.fit_transform(y_train).reshape(-1)
y_test = output_scaler.transform(y_test).reshape(-1)

dataset_copy = [x_train.copy(), x_test.copy(), y_train.copy(), y_test.copy()]

# Trainin gng
num_epochs = 2000
train_loss_history = []
test_loss_history = []
w_history = []
dim = x_train.shape[0]
num_train = x_train.shape[1]
num_test = x_test.shape[1]

model = LinearRegression(dim=dim, lr=0.1)
for i in range(num_epochs):
    y_hat = model.forward(x_train)
    train_loss = 1 / (2 * num_train) * ((y_train - y_hat) ** 2).sum()

    w_history.append(model.w.copy())
    model.backward(x_train, y_hat, y_train)
    model.optimize()

    y_hat = model.forward(x_test)
    test_loss = 1 / (2 * num_test) * ((y_test - y_hat) ** 2).sum()

    train_loss_history.append(train_loss)
    test_loss_history.append(test_loss)

    if i % 200 == 0:
        print(f"Epoch {i} | Train Loss {train_loss} | Test loss {test_loss}")

y_test_orig = output_scaler.inverse_transform(y_test[np.newaxis, :]).ravel()
y_hat_orig = output_scaler.inverse_transform(y_hat[np.newaxis, :]).ravel()

print("MSE:", mse(y_test_orig, y_hat_orig))
print("MAE:", mae(y_test_orig, y_hat_orig))
print("R^2 :", r2_score(y_test_orig, y_hat_orig))

plt.figure()
plt.scatter(y_test_orig, y_hat_orig)
plt.plot([y_test_orig.min(), y_test_orig.max()], [y_test_orig.min(), y_test_orig.max()])
plt.xlabel("Actual Temperature")
plt.ylabel("Predicted Temperature")
plt.title("Actual vs Predicted Temperature")
plt.savefig("actual_vs_predicted.png", dpi=150)
# plt.show()
plt.close()
