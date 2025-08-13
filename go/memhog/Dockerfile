# syntax=docker/dockerfile:1.4
ARG GO_VERSION=1.24
FROM --platform=$BUILDPLATFORM golang:$GO_VERSION AS builder
WORKDIR /src
COPY go.mod go.sum ./
RUN go mod download
COPY . .
ARG TARGETOS
ARG TARGETARCH
RUN CGO_ENABLED=0 GOOS=${TARGETOS} GOARCH=${TARGETARCH} go build -o /memhog .
FROM --platform=$TARGETPLATFORM scratch AS final
WORKDIR /
COPY --from=builder /memhog .
ENTRYPOINT ["/memhog"]

