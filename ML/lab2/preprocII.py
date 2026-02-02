import pandas as pd
from sklearn.preprocessing import MinMaxScaler

def getBounds(ser: pd.Series):
    q1    = ser.quantile(q=.25)
    q3    = ser.quantile(q=.75)
    iqr   = q3 - q1
    low_b = q1 - 1.5 * iqr
    up_b  = q3 + 1.5 * iqr
    return (low_b, up_b)

df = pd.read_excel("Heart.xlsx")
print("A:")
print(df.head(5))
print("")
print(df.tail(5))
print("")

print("B:")
df.columns = list(map(lambda x: x.lower(), df.columns))
print(df.columns)
print("")

print("C: ")
print(df[df.duplicated() == True])
df = df.drop_duplicates()
print("")

print("D: Outliers in Cholestrol")
chol = df["chol"]
low_b, up_b  = getBounds(chol)
print(chol[((chol < low_b) | (chol > (up_b)))])
print("")

print("E: Outliers in RestBP:")
restbp = df["restbp"]
medi = restbp.median()
low_b, up_b = getBounds(restbp)
print(restbp[((restbp < low_b) | (restbp > (up_b)))])
restbp[((restbp < low_b) | (restbp > (up_b)))] = medi
print("Median:", medi, "\nOutliers After replacing with median:")
print(restbp[((restbp < low_b) | (restbp > (up_b)))])
print("")

print("F: Categorical Cols")
df["chestpain"] = pd.factorize(df["chestpain"])[0]
df["ahd"] = pd.factorize(df["ahd"])[0]
df["thal"] = pd.factorize(df["thal"])[0]
print(df.head())
print("")

print("G: Min Max Normalization")
scaler = MinMaxScaler()
minmax_df = scaler.fit_transform(df)
minmax_df = pd.DataFrame(minmax_df, columns=df.columns)
print(minmax_df)
print("")
